// Contract tests that run the same scenario against every renderer backend that is
// compiled in and usable on this machine. They need a display and a GPU (or a software
// implementation) and are skipped otherwise.

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "JadeFrame/base_app.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/graphics/shader_loader.h"

using namespace JadeFrame;

namespace {

struct RGB {
    u8 r;
    u8 g;
    u8 b;
};

// Only fully saturated colors are used. They are identical in linear and sRGB encoding,
// so the tests do not depend on the color space of the window's framebuffer.
constexpr RGB CLEAR = {0, 0, 255};
constexpr RGB QUAD = {255, 0, 0};

// Row 0 is the bottom row, like in `Image`.
auto pixel(const Image& image, i32 x, i32 y) -> RGB {
    const size_t i = (static_cast<size_t>(y) * static_cast<size_t>(image.width) +
                      static_cast<size_t>(x)) *
                     static_cast<size_t>(image.num_components);
    return RGB{image.data[i], image.data[i + 1], image.data[i + 2]};
}

auto expect_color(const Image& image, i32 x, i32 y, RGB expected, const char* what)
    -> void {
    const RGB actual = pixel(image, x, y);
    const i32 tolerance = 2;
    EXPECT_NEAR(actual.r, expected.r, tolerance) << what << " at " << x << "," << y;
    EXPECT_NEAR(actual.g, expected.g, tolerance) << what << " at " << x << "," << y;
    EXPECT_NEAR(actual.b, expected.b, tolerance) << what << " at " << x << "," << y;
}

// `Instance` is a process-wide singleton that cannot be created twice, and it owns the
// windows. Both are therefore shared by all tests: one window per backend, each test
// creates its own renderer on it.
auto shared_instance() -> Instance& {
    static auto* instance = new Instance();
    return *instance;
}

auto shared_window(GRAPHICS_API api) -> Window* {
    static std::map<GRAPHICS_API, Window*> windows;
    auto                                   it = windows.find(api);
    if (it == windows.end()) {
        Window* window = shared_instance().m_system_manager.request_window(Window::Desc{
            .title = std::string("Renderer contract - ") + to_string(api),
            .size = v2u32::create(320, 240),
            .position = v2u32::create(0, 0),
        });
        it = windows.emplace(api, window).first;
    }
    return it->second;
}

class RendererContract : public testing::TestWithParam<GRAPHICS_API> {
protected:
    auto SetUp() -> void override {
        const GRAPHICS_API api = GetParam();

        bool available = false;
        for (GRAPHICS_API candidate : RenderSystem::list_available_graphics_apis()) {
            available = available || candidate == api;
        }
        if (!available) { GTEST_SKIP() << to_string(api) << " is not available"; }
        if (std::getenv("DISPLAY") == nullptr) { GTEST_SKIP() << "No display"; }

        try {
            m_window = shared_window(api);
        } catch (const std::exception& e) {
            GTEST_SKIP() << "Cannot create a window for " << to_string(api) << ": "
                         << e.what();
        }
        m_system = std::make_unique<RenderSystem>();
        auto initialized = m_system->init(api, m_window);
        if (initialized.is_err()) {
            m_system.reset();
            GTEST_SKIP() << "Cannot create a " << to_string(api) << " renderer: "
                         << to_string(initialized.error());
        }
        ASSERT_TRUE(m_system->is_initialized());
        ASSERT_NE(m_system->m_renderer, nullptr);
        m_renderer = m_system->m_renderer.get();
        m_renderer->set_clear_color(RGBAColor::from_rgba(0.0F, 0.0F, 1.0F, 1.0F));

        ShaderHandle::Desc shader_desc;
        auto shading_code = GLSLCodeLoader::get_by_name("spirv_test_1");
        ASSERT_TRUE(shading_code.is_ok()) << shading_code.error().m_message;
        shader_desc.shading_code = std::move(shading_code).value();
        m_shader = m_system->register_shader(shader_desc);
        ASSERT_NE(m_shader, nullptr);
        ASSERT_NE(m_shader->m_handle, nullptr);
        MaterialHandle* material = m_system->register_material(m_shader, nullptr);
        ASSERT_NE(material, nullptr);

        // A red quad that covers the lower-left quarter of clip space.
        const std::vector<v3> positions = {
            v3::create(-1.0F, -1.0F, 0.0F), v3::create(0.0F, -1.0F, 0.0F),
            v3::create(0.0F, 0.0F, 0.0F),   v3::create(-1.0F, -1.0F, 0.0F),
            v3::create(0.0F, 0.0F, 0.0F),   v3::create(-1.0F, 0.0F, 0.0F),
        };
        const std::vector<RGBAColor> colors(positions.size(), RGBAColor::solid_red());
        m_mesh.m_attributes[Mesh::POSITION.m_id] = Mesh::AttributeData{
            .m_attribute = Mesh::POSITION, .m_data = to_list(positions)
        };
        m_mesh.m_attributes[Mesh::COLOR.m_id] =
            Mesh::AttributeData{.m_attribute = Mesh::COLOR, .m_data = to_list(colors)};

        m_object.m_mesh = m_system->register_mesh(m_mesh);
        ASSERT_NE(m_object.m_mesh, nullptr);
        m_object.m_vertex_data = &m_mesh;
        m_object.m_material = material;

        // Let the window settle (a window manager may resize it) before the tests look
        // at sizes.
        for (i32 i = 0; i < 5; ++i) {
            render_frame();
            m_renderer->present();
            poll_events();
        }
    }

    auto TearDown() -> void override { m_system.reset(); }

    auto poll_events() -> void {
        bool running = true;
        m_window->handle_events(running);
    }

    // Leaves the frame rendered and not presented.
    auto render_frame() -> void {
        m_renderer->clear_background();
        m_system->submit(m_object);
        m_renderer->render(m_camera);
    }

    // Renders frames until one can be captured. The first frames after a resize may be
    // dropped while a swapchain is recreated.
    auto render_and_capture(const char* filename = nullptr) -> Image {
        for (i32 attempt = 0; attempt < 5; ++attempt) {
            render_frame();
            auto screenshot = m_renderer->take_screenshot(filename);
            m_renderer->present();
            poll_events();
            if (screenshot.is_ok()) { return std::move(screenshot).value(); }
            EXPECT_EQ(screenshot.error().m_code, RendererErrorCode::FRAME_NOT_AVAILABLE)
                << screenshot.error().m_message;
        }
        ADD_FAILURE() << "No frame could be captured";
        return {};
    }

    Window*                       m_window = nullptr;
    std::unique_ptr<RenderSystem> m_system;
    IRenderer*                    m_renderer = nullptr;
    ShaderHandle*                 m_shader = nullptr;
    Mesh                          m_mesh;
    Object                        m_object;
    Camera m_camera = Camera::orthographic(-1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F);
};

TEST_P(RendererContract, NamedShaderUniformsAreReportedAsUnsupported) {
    const f32 value = 1.0F;

    auto direct = m_renderer->set_shader_uniform(
        m_shader->m_handle.get(), "u_value", &value, sizeof(value)
    );
    ASSERT_TRUE(direct.is_err());
    EXPECT_EQ(direct.error().m_code, RendererErrorCode::UNSUPPORTED);

    auto through_handle = m_shader->set_uniform("u_value", &value, sizeof(value));
    ASSERT_TRUE(through_handle.is_err());
    EXPECT_EQ(through_handle.error().m_code, RendererErrorCode::UNSUPPORTED);
}

TEST_P(RendererContract, UnusableShaderCodeYieldsANullHandle) {
    ShaderHandle::Desc desc;
    desc.shading_code.m_shading_language = SHADING_LANGUAGE::SPIRV_VULKAN;

    ShaderHandle* shader = m_system->register_shader(desc);

    ASSERT_NE(shader, nullptr);
    EXPECT_EQ(shader->m_handle, nullptr);
    EXPECT_EQ(m_system->register_material(shader, nullptr), nullptr);
}

TEST_P(RendererContract, ScreenshotNeedsARenderedUnpresentedFrame) {
    // The last frame of SetUp was presented.
    auto before = m_renderer->take_screenshot(nullptr);
    ASSERT_TRUE(before.is_err());
    EXPECT_EQ(before.error().m_code, RendererErrorCode::FRAME_NOT_AVAILABLE);

    const Image image = render_and_capture();
    EXPECT_FALSE(image.data.empty());

    auto after = m_renderer->take_screenshot(nullptr);
    ASSERT_TRUE(after.is_err());
    EXPECT_EQ(after.error().m_code, RendererErrorCode::FRAME_NOT_AVAILABLE);
}

TEST_P(RendererContract, FullViewportScreenshotHasWindowSizeAndBottomUpRows) {
    const v2u32 size = m_window->get_size();
    m_renderer->set_viewport(0, 0, size.x, size.y);

    const Image image = render_and_capture();

    ASSERT_EQ(image.width, static_cast<i32>(size.x));
    ASSERT_EQ(image.height, static_cast<i32>(size.y));
    ASSERT_EQ(image.num_components, 3);
    ASSERT_EQ(
        image.data.size(), static_cast<size_t>(image.width) * image.height * 3U
    );

    const i32 w = image.width;
    const i32 h = image.height;
    // The quad is in the lower-left quarter, which is at the start of the data.
    expect_color(image, w / 4, h / 4, QUAD, "lower left");
    expect_color(image, 1, 1, QUAD, "lower left corner");
    expect_color(image, 3 * w / 4, h / 4, CLEAR, "lower right");
    expect_color(image, w / 4, 3 * h / 4, CLEAR, "upper left");
    expect_color(image, 3 * w / 4, 3 * h / 4, CLEAR, "upper right");
    expect_color(image, w - 2, h - 2, CLEAR, "upper right corner");
}

TEST_P(RendererContract, ViewportMovesAndScalesTheRenderedImage) {
    const v2u32 size = m_window->get_size();
    // The upper-right quarter of the window.
    const u32 x = size.x / 2;
    const u32 y = size.y / 2;
    const u32 width = size.x - x;
    const u32 height = size.y - y;
    m_renderer->set_viewport(x, y, width, height);

    const Image part = render_and_capture();

    // The screenshot is the viewport rectangle; clip space is mapped into it.
    ASSERT_EQ(part.width, static_cast<i32>(width));
    ASSERT_EQ(part.height, static_cast<i32>(height));
    expect_color(part, part.width / 4, part.height / 4, QUAD, "lower left of viewport");
    expect_color(part, 3 * part.width / 4, part.height / 4, CLEAR, "lower right");
    expect_color(part, part.width / 4, 3 * part.height / 4, CLEAR, "upper left");
    expect_color(part, 3 * part.width / 4, 3 * part.height / 4, CLEAR, "upper right");

    // Looking at the whole window afterwards shows where the viewport was: the quad
    // must be gone from the lower-left quarter of the window.
    m_renderer->set_viewport(0, 0, size.x, size.y);
    const Image full = render_and_capture();
    ASSERT_EQ(full.width, static_cast<i32>(size.x));
    expect_color(full, full.width / 4, full.height / 4, QUAD, "full viewport restored");
}

TEST_P(RendererContract, ViewportBeyondTheFramebufferIsClipped) {
    const v2u32 size = m_window->get_size();
    // Twice the window: the quad's quarter of clip space now fills the whole window.
    m_renderer->set_viewport(0, 0, size.x * 2, size.y * 2);

    const Image image = render_and_capture();

    ASSERT_EQ(image.width, static_cast<i32>(size.x));
    ASSERT_EQ(image.height, static_cast<i32>(size.y));
    expect_color(image, 1, 1, QUAD, "lower left corner");
    expect_color(image, image.width - 2, image.height - 2, QUAD, "upper right corner");
}

// The interface has no way to resize a window, so this relies on the `wmctrl` tool and
// a window manager that honors it. It is skipped where that does not work.
TEST_P(RendererContract, FullViewportFollowsAWindowResize) {
    const v2u32       before = m_window->get_size();
    const v2u32       target = v2u32::create(before.x + 180, before.y + 140);
    const std::string command =
        std::string("wmctrl -r 'Renderer contract - ") + to_string(GetParam()) +
        "' -e 0,-1,-1," + std::to_string(target.x) + "," + std::to_string(target.y) +
        " 2>/dev/null";
    if (std::system(command.c_str()) != 0) { GTEST_SKIP() << "wmctrl is not usable"; }

    v2u32 size = before;
    for (i32 i = 0; i < 200 && size.x == before.x && size.y == before.y; ++i) {
        render_frame();
        m_renderer->present();
        poll_events();
        size = m_window->get_size();
    }
    if (size.x == before.x && size.y == before.y) {
        GTEST_SKIP() << "The window manager did not resize the window";
    }

    // What `Application` does when it receives the resize event.
    m_renderer->set_viewport(0, 0, size.x, size.y);
    // A backend may need a few frames until its framebuffer has the new size (Vulkan
    // recreates its swapchain when presenting fails). Until then a screenshot is
    // clipped to the old framebuffer.
    Image image = render_and_capture();
    for (i32 i = 0; i < 10 && image.width != static_cast<i32>(size.x); ++i) {
        image = render_and_capture();
    }

    ASSERT_EQ(image.width, static_cast<i32>(size.x));
    ASSERT_EQ(image.height, static_cast<i32>(size.y));
    const i32 w = image.width;
    const i32 h = image.height;
    // The quad still covers exactly the lower-left quarter of the resized window.
    expect_color(image, w / 4, h / 4, QUAD, "lower left");
    expect_color(image, w / 2 - 3, h / 2 - 3, QUAD, "inside the quad's corner");
    expect_color(image, w / 2 + 3, h / 2 + 3, CLEAR, "outside the quad's corner");
    expect_color(image, 3 * w / 4, 3 * h / 4, CLEAR, "upper right");
    expect_color(image, w - 2, h - 2, CLEAR, "upper right corner");
}

TEST_P(RendererContract, EmptyViewportRendersButCannotBeCaptured) {
    m_renderer->set_viewport(0, 0, 0, 0);

    render_frame();
    auto screenshot = m_renderer->take_screenshot(nullptr);
    m_renderer->present();

    ASSERT_TRUE(screenshot.is_err());
    EXPECT_EQ(screenshot.error().m_code, RendererErrorCode::INVALID_STATE);
}

TEST_P(RendererContract, ScreenshotIsWrittenToTheGivenFile) {
    const v2u32 size = m_window->get_size();
    m_renderer->set_viewport(0, 0, size.x, size.y);
    const std::string path =
        (std::filesystem::temp_directory_path() /
         (std::string("jadeframe_contract_") + to_string(GetParam()) + ".png"))
            .string();
    std::filesystem::remove(path);

    const Image image = render_and_capture(path.c_str());

    ASSERT_TRUE(std::filesystem::exists(path));
    const Image loaded = Image::load_from_path(path);
    if (std::getenv("JADEFRAME_KEEP_TEST_IMAGES") == nullptr) {
        std::filesystem::remove(path);
    }
    ASSERT_EQ(loaded.width, image.width);
    ASSERT_EQ(loaded.height, image.height);
    expect_color(loaded, loaded.width / 4, loaded.height / 4, QUAD, "file lower left");
    expect_color(
        loaded, 3 * loaded.width / 4, 3 * loaded.height / 4, CLEAR, "file upper right"
    );
}

TEST_P(RendererContract, ScreenshotToAnUnwritablePathIsAnIoFailure) {
    const v2u32 size = m_window->get_size();
    m_renderer->set_viewport(0, 0, size.x, size.y);

    render_frame();
    auto screenshot =
        m_renderer->take_screenshot("/nonexistent-directory/jadeframe.png");
    m_renderer->present();

    ASSERT_TRUE(screenshot.is_err());
    EXPECT_EQ(screenshot.error().m_code, RendererErrorCode::IO_FAILURE);
}

auto compiled_backends() -> std::vector<GRAPHICS_API> {
    std::vector<GRAPHICS_API> apis;
#if JADEFRAME_ENABLE_OPENGL
    apis.push_back(GRAPHICS_API::OPENGL);
#endif
#if JADEFRAME_ENABLE_VULKAN
    apis.push_back(GRAPHICS_API::VULKAN);
#endif
    return apis;
}

INSTANTIATE_TEST_SUITE_P(
    Backends,
    RendererContract,
    testing::ValuesIn(compiled_backends()),
    [](const testing::TestParamInfo<GRAPHICS_API>& info) {
        return std::string(to_string(info.param));
    }
);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(RendererContract);

} // namespace
