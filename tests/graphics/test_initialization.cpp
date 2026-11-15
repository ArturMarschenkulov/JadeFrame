// Initialization failures and the cleanup after them. Nothing in here needs a display
// or a GPU: backends are either rejected before they touch the graphics API, driven
// with a window that has no native window, or replaced by a fake renderer.

#include <gtest/gtest.h>

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "JadeFrame/base_app.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/graphics/renderer_factory.h"
#include "JadeFrame/platform/window.h"

using namespace JadeFrame;

namespace {

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

auto apis_without_renderer() -> std::vector<GRAPHICS_API> {
    return {
        GRAPHICS_API::UNDEFINED, GRAPHICS_API::D3D11,    GRAPHICS_API::D3D12,
        GRAPHICS_API::METAL,     GRAPHICS_API::SOFTWARE, GRAPHICS_API::TERMINAL,
    };
}

// What happened to the fake renderers and their resources, in order.
auto events() -> std::vector<std::string>& {
    static std::vector<std::string> log;
    return log;
}

auto count(const std::string& event) -> i32 {
    i32 result = 0;
    for (const std::string& e : events()) { result += e == event ? 1 : 0; }
    return result;
}

auto index_of(const std::string& event) -> i32 {
    for (size_t i = 0; i < events().size(); ++i) {
        if (events()[i] == event) { return static_cast<i32>(i); }
    }
    return -1;
}

auto delete_fake_shader(void* handle) noexcept -> void {
    events().emplace_back("shader released");
    delete static_cast<i32*>(handle);
}

// A renderer whose `initialize()` outcome is chosen by the test. Every operation other
// than `initialize()` and destruction records whether it was used without a successful
// initialization.
class FakeRenderer : public IRenderer {
public:
    FakeRenderer(std::string name, bool initialization_succeeds)
        : m_name(std::move(name))
        , m_initialization_succeeds(initialization_succeeds) {
        events().push_back(m_name + " constructed");
    }
    ~FakeRenderer() override { events().push_back(m_name + " destroyed"); }

    auto initialize() -> Result<void, InitError> override {
        events().push_back(m_name + " initialize");
        if (!m_initialization_succeeds) {
            return err<void, InitError>(InitError{
                InitErrorCode::RESOURCE_CREATION_FAILED,
                GRAPHICS_API::SOFTWARE,
                "fake framebuffer is incomplete",
            });
        }
        m_initialized = true;
        return ok<void, InitError>();
    }

    auto create_buffer(GPUBuffer::TYPE /*type*/, const void* /*data*/, size_t /*size*/)
        -> void* override {
        used();
        return nullptr;
    }
    auto destroy_buffer(void* /*handle*/) -> void override { used(); }
    auto create_texture(const Image& /*image*/) -> NativeHandle override {
        used();
        return {nullptr, noop_native_handle_deleter};
    }
    auto create_shader(const ShadingCode& /*code*/) -> NativeHandle override {
        used();
        return {new i32(0), delete_fake_shader};
    }
    auto create_material(const ShaderHandle& /*shader*/, const TextureHandle* /*texture*/)
        -> NativeHandle override {
        used();
        return {nullptr, noop_native_handle_deleter};
    }
    auto set_shader_uniform(
        void* /*shader*/,
        const std::string& /*name*/,
        const void* /*data*/,
        size_t /*size*/
    ) -> Result<void, RendererError> override {
        used();
        return err<void, RendererError>(RendererError{
            RendererErrorCode::UNSUPPORTED, "fake"
        });
    }
    auto set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/)
        -> void override {
        used();
    }
    auto take_screenshot(const char* /*filename*/)
        -> Result<Image, RendererError> override {
        used();
        return err<Image, RendererError>(RendererError{
            RendererErrorCode::UNSUPPORTED, "fake"
        });
    }
    auto set_clear_color(const RGBAColor& /*color*/) -> void override { used(); }
    auto clear_background() -> void override { used(); }
    auto render(const Camera& /*cam*/) -> void override { used(); }
    auto present() -> void override { used(); }
    auto wait_until_idle() -> void override {
        used();
        events().push_back(m_name + " wait_until_idle");
    }

private:
    auto used() -> void {
        if (!m_initialized) { events().push_back(m_name + " used uninitialized"); }
    }

    std::string m_name;
    bool        m_initialization_succeeds;
    bool        m_initialized = false;
};

class Initialization : public testing::Test {
protected:
    auto SetUp() -> void override { events().clear(); }
};

using InitErrorTest = Initialization;
using RendererFactory = Initialization;
using GraphicsContext = Initialization;
using RenderSystemInit = Initialization;
using ApplicationStartup = Initialization;

// `Instance` is a process-wide singleton that cannot be created twice.
auto shared_instance() -> Instance& {
    static auto* instance = new Instance();
    return *instance;
}

// Counts what `Application::start()` invoked.
struct Callbacks {
    i32 init = 0;
    i32 update = 0;
    i32 draw = 0;
    i32 frame_rendered = 0;

    auto attach(Application& app) -> void {
        app.m_on_init_fn = [this] { ++init; };
        app.m_on_update_fn = [this] { ++update; };
        app.m_on_draw_fn = [this] { ++draw; };
        app.m_on_frame_rendered_fn = [this] { ++frame_rendered; };
    }

    [[nodiscard]] auto total() const -> i32 {
        return init + update + draw + frame_rendered;
    }
};

auto expect_not_started(const Application& app, const Callbacks& callbacks) -> void {
    EXPECT_EQ(callbacks.total(), 0) << "a callback ran although startup failed";
    EXPECT_EQ(app.m_tick, 0U) << "the main loop was entered";
    EXPECT_FALSE(app.m_render_system.is_initialized());
    EXPECT_EQ(app.m_render_system.m_renderer, nullptr);
    EXPECT_TRUE(app.m_windows.empty());
    EXPECT_FALSE(app.m_gui.m_is_initialized);
}

} // namespace

TEST_F(InitErrorTest, EveryCodeHasAName) {
    EXPECT_STREQ(to_string(InitErrorCode::INVALID_ARGUMENT), "INVALID_ARGUMENT");
    EXPECT_STREQ(to_string(InitErrorCode::NOT_INITIALIZED), "NOT_INITIALIZED");
    EXPECT_STREQ(
        to_string(InitErrorCode::WINDOW_CREATION_FAILED), "WINDOW_CREATION_FAILED"
    );
    EXPECT_STREQ(to_string(InitErrorCode::BACKEND_UNSUPPORTED), "BACKEND_UNSUPPORTED");
    EXPECT_STREQ(to_string(InitErrorCode::BACKEND_DISABLED), "BACKEND_DISABLED");
    EXPECT_STREQ(
        to_string(InitErrorCode::CONTEXT_CREATION_FAILED), "CONTEXT_CREATION_FAILED"
    );
    EXPECT_STREQ(
        to_string(InitErrorCode::RESOURCE_CREATION_FAILED), "RESOURCE_CREATION_FAILED"
    );
}

TEST_F(InitErrorTest, DescriptionNamesCodeApiAndCause) {
    const InitError with_api{
        InitErrorCode::CONTEXT_CREATION_FAILED, GRAPHICS_API::VULKAN, "no device"
    };
    EXPECT_EQ(to_string(with_api), "CONTEXT_CREATION_FAILED [Vulkan]: no device");

    const InitError without_api{
        InitErrorCode::NOT_INITIALIZED, GRAPHICS_API::UNDEFINED, "nothing to start"
    };
    EXPECT_EQ(to_string(without_api), "NOT_INITIALIZED: nothing to start");
}

// --- Unsupported or disabled graphics backends, renderer factory failures ---

TEST_F(RendererFactory, ApisWithoutARendererAreUnsupported) {
    RenderSystem system;
    Window       window;
    for (GRAPHICS_API api : apis_without_renderer()) {
        SCOPED_TRACE(to_string(api));

        auto support = check_renderer_support(api);
        ASSERT_TRUE(support.is_err());
        EXPECT_EQ(support.error().m_code, InitErrorCode::BACKEND_UNSUPPORTED);
        EXPECT_EQ(support.error().m_api, api);
        EXPECT_FALSE(support.error().m_message.empty());

        auto renderer = create_renderer(api, system, &window);
        ASSERT_TRUE(renderer.is_err());
        EXPECT_EQ(renderer.error().m_code, InitErrorCode::BACKEND_UNSUPPORTED);

        auto through_system = RenderSystem::check_graphics_api(api);
        ASSERT_TRUE(through_system.is_err());
        EXPECT_EQ(through_system.error().m_code, InitErrorCode::BACKEND_UNSUPPORTED);
    }
}

TEST_F(RendererFactory, AValueThatIsNoGraphicsApiIsUnsupported) {
    auto support = check_renderer_support(static_cast<GRAPHICS_API>(200));

    ASSERT_TRUE(support.is_err());
    EXPECT_EQ(support.error().m_code, InitErrorCode::BACKEND_UNSUPPORTED);
}

TEST_F(RendererFactory, BackendsDisabledAtBuildTimeAreReportedAsDisabled) {
    const auto expect = [](GRAPHICS_API api, bool enabled) {
        SCOPED_TRACE(to_string(api));
        RenderSystem system;
        Window       window;

        auto support = check_renderer_support(api);
        auto renderer = create_renderer(api, system, &window);
        if (enabled) {
            EXPECT_TRUE(support.is_ok());
            EXPECT_TRUE(renderer.is_ok());
            return;
        }
        ASSERT_TRUE(support.is_err());
        EXPECT_EQ(support.error().m_code, InitErrorCode::BACKEND_DISABLED);
        EXPECT_EQ(support.error().m_api, api);
        ASSERT_TRUE(renderer.is_err());
        EXPECT_EQ(renderer.error().m_code, InitErrorCode::BACKEND_DISABLED);

        // The render system reports the same error and does not pick another backend.
        auto initialized = system.init(api, &window);
        ASSERT_TRUE(initialized.is_err());
        EXPECT_EQ(initialized.error().m_code, InitErrorCode::BACKEND_DISABLED);
        EXPECT_EQ(system.m_renderer, nullptr);
    };
    expect(GRAPHICS_API::OPENGL, JADEFRAME_ENABLE_OPENGL != 0);
    expect(GRAPHICS_API::VULKAN, JADEFRAME_ENABLE_VULKAN != 0);
}

TEST_F(RendererFactory, ARendererCannotBeCreatedWithoutAWindow) {
    for (GRAPHICS_API api : compiled_backends()) {
        SCOPED_TRACE(to_string(api));
        RenderSystem system;

        auto renderer = create_renderer(api, system, nullptr);

        ASSERT_TRUE(renderer.is_err());
        EXPECT_EQ(renderer.error().m_code, InitErrorCode::INVALID_ARGUMENT);
        EXPECT_EQ(renderer.error().m_api, api);
    }
}

TEST_F(RendererFactory, CreatedRendererIsNotNullAndCanBeDestroyedUninitialized) {
    for (GRAPHICS_API api : compiled_backends()) {
        SCOPED_TRACE(to_string(api));
        RenderSystem system;
        Window       window;

        auto renderer = create_renderer(api, system, &window);

        ASSERT_TRUE(renderer.is_ok());
        EXPECT_NE(renderer.value(), nullptr);
        // Going out of scope destroys a renderer that was never initialized.
    }
}

TEST_F(RenderSystemInit, UnsupportedApiLeavesNoRenderer) {
    Window window;
    for (GRAPHICS_API api : apis_without_renderer()) {
        SCOPED_TRACE(to_string(api));
        RenderSystem system;

        auto initialized = system.init(api, &window);

        ASSERT_TRUE(initialized.is_err());
        EXPECT_EQ(initialized.error().m_code, InitErrorCode::BACKEND_UNSUPPORTED);
        EXPECT_FALSE(system.is_initialized());
        EXPECT_EQ(system.m_renderer, nullptr);
        EXPECT_EQ(system.m_api, GRAPHICS_API::UNDEFINED);
    }
}

TEST_F(RenderSystemInit, MissingWindowIsAnError) {
    for (GRAPHICS_API api : compiled_backends()) {
        SCOPED_TRACE(to_string(api));
        RenderSystem system;

        auto initialized = system.init(api, nullptr);

        ASSERT_TRUE(initialized.is_err());
        EXPECT_EQ(initialized.error().m_code, InitErrorCode::INVALID_ARGUMENT);
        EXPECT_EQ(system.m_renderer, nullptr);
    }
}

// --- Graphics context initialization failures ---

// A window without a native window cannot get a graphics context. OpenGL fails before
// it touches GLX. Vulkan fails either while creating its instance or, where a Vulkan
// loader is installed, when creating the surface - after the instance was created, so
// that this also runs the cleanup of a partially initialized context.
TEST_F(GraphicsContext, CreationFailureIsReportedAndCleanedUp) {
    for (GRAPHICS_API api : compiled_backends()) {
        SCOPED_TRACE(to_string(api));
        Window       window;
        RenderSystem system;

        auto initialized = system.init(api, &window);

        ASSERT_TRUE(initialized.is_err());
        EXPECT_EQ(initialized.error().m_code, InitErrorCode::CONTEXT_CREATION_FAILED)
            << to_string(initialized.error());
        EXPECT_EQ(initialized.error().m_api, api);
        EXPECT_FALSE(initialized.error().m_message.empty());
        EXPECT_FALSE(system.is_initialized());
        EXPECT_EQ(system.m_renderer, nullptr);
        EXPECT_EQ(system.m_api, GRAPHICS_API::UNDEFINED);

        // The failed attempt left nothing behind that would break another one or the
        // shutdown.
        auto again = system.init(api, &window);
        EXPECT_TRUE(again.is_err());
        system.shutdown();
    }
}

// --- Required renderer resource failures, cleanup after partial initialization ---

TEST_F(RenderSystemInit, RendererIsRequired) {
    RenderSystem system;

    auto initialized = system.init_with_renderer(GRAPHICS_API::SOFTWARE, nullptr);

    ASSERT_TRUE(initialized.is_err());
    EXPECT_EQ(initialized.error().m_code, InitErrorCode::INVALID_ARGUMENT);
    EXPECT_FALSE(system.is_initialized());
}

TEST_F(RenderSystemInit, FailedRendererInitializationIsPropagatedAndCleanedUp) {
    {
        RenderSystem system;

        auto initialized = system.init_with_renderer(
            GRAPHICS_API::SOFTWARE, std::make_unique<FakeRenderer>("a", false)
        );

        ASSERT_TRUE(initialized.is_err());
        EXPECT_EQ(initialized.error().m_code, InitErrorCode::RESOURCE_CREATION_FAILED);
        EXPECT_EQ(initialized.error().m_message, "fake framebuffer is incomplete");

        // The renderer is already gone when the call returns.
        EXPECT_EQ(count("a initialize"), 1);
        EXPECT_EQ(count("a destroyed"), 1);
        EXPECT_FALSE(system.is_initialized());
        EXPECT_EQ(system.m_renderer, nullptr);
        EXPECT_EQ(system.m_api, GRAPHICS_API::UNDEFINED);

        // Nothing can reach the failed renderer.
        Image image;
        EXPECT_EQ(system.register_texture(image), nullptr);
        ShaderHandle* shader = system.register_shader(ShaderHandle::Desc{});
        ASSERT_NE(shader, nullptr);
        EXPECT_EQ(shader->m_handle, nullptr);
        EXPECT_EQ(system.register_material(shader, nullptr), nullptr);

        system.shutdown();
        system.shutdown();
    }
    // Neither the shutdowns nor the destructor touched or destroyed it again.
    EXPECT_EQ(count("a destroyed"), 1);
    EXPECT_EQ(count("a wait_until_idle"), 0);
    EXPECT_EQ(count("a used uninitialized"), 0);
}

TEST_F(RenderSystemInit, SuccessfulInitializationAndShutdown) {
    {
        RenderSystem system;

        auto initialized = system.init_with_renderer(
            GRAPHICS_API::SOFTWARE, std::make_unique<FakeRenderer>("a", true)
        );

        ASSERT_TRUE(initialized.is_ok());
        EXPECT_TRUE(system.is_initialized());
        ASSERT_NE(system.m_renderer, nullptr);
        EXPECT_EQ(system.m_api, GRAPHICS_API::SOFTWARE);
        EXPECT_EQ(count("a initialize"), 1);
        EXPECT_EQ(count("a destroyed"), 0);

        ShaderHandle* shader = system.register_shader(ShaderHandle::Desc{});
        ASSERT_NE(shader, nullptr);
        EXPECT_NE(shader->m_handle, nullptr);

        system.shutdown();

        EXPECT_FALSE(system.is_initialized());
        EXPECT_EQ(system.m_api, GRAPHICS_API::UNDEFINED);
        EXPECT_TRUE(system.m_registered_shaders.empty());
        EXPECT_EQ(count("a wait_until_idle"), 1);
        EXPECT_EQ(count("a destroyed"), 1);
        // Resources are released through the renderer, so they have to go first.
        ASSERT_EQ(count("shader released"), 1);
        EXPECT_LT(index_of("a wait_until_idle"), index_of("shader released"));
        EXPECT_LT(index_of("shader released"), index_of("a destroyed"));

        system.shutdown();
    }
    EXPECT_EQ(count("a destroyed"), 1);
    EXPECT_EQ(count("a wait_until_idle"), 1);
    EXPECT_EQ(count("a used uninitialized"), 0);
}

TEST_F(RenderSystemInit, DestructorShutsDownAnInitializedSystem) {
    {
        RenderSystem system;
        ASSERT_TRUE(system
                        .init_with_renderer(
                            GRAPHICS_API::SOFTWARE,
                            std::make_unique<FakeRenderer>("a", true)
                        )
                        .is_ok());
        ASSERT_NE(system.register_shader(ShaderHandle::Desc{}), nullptr);
    }
    EXPECT_EQ(count("a destroyed"), 1);
    ASSERT_EQ(count("shader released"), 1);
    EXPECT_LT(index_of("shader released"), index_of("a destroyed"));
}

TEST_F(RenderSystemInit, FailedReinitializationReleasesThePreviousRendererToo) {
    RenderSystem system;
    ASSERT_TRUE(system
                    .init_with_renderer(
                        GRAPHICS_API::SOFTWARE, std::make_unique<FakeRenderer>("a", true)
                    )
                    .is_ok());
    ASSERT_NE(system.register_shader(ShaderHandle::Desc{}), nullptr);

    auto second = system.init_with_renderer(
        GRAPHICS_API::SOFTWARE, std::make_unique<FakeRenderer>("b", false)
    );

    ASSERT_TRUE(second.is_err());
    // The old renderer was shut down before the new one was initialized, and the
    // system did not fall back to it.
    EXPECT_LT(index_of("a destroyed"), index_of("b initialize"));
    EXPECT_EQ(count("a destroyed"), 1);
    EXPECT_EQ(count("shader released"), 1);
    EXPECT_EQ(count("b destroyed"), 1);
    EXPECT_FALSE(system.is_initialized());
    EXPECT_TRUE(system.m_registered_shaders.empty());

    // A later attempt can still succeed.
    auto third = system.init_with_renderer(
        GRAPHICS_API::SOFTWARE, std::make_unique<FakeRenderer>("c", true)
    );
    ASSERT_TRUE(third.is_ok());
    EXPECT_TRUE(system.is_initialized());
    EXPECT_EQ(count("c destroyed"), 0);
    EXPECT_EQ(count("b used uninitialized"), 0);
}

// --- Application startup failure propagation ---

TEST_F(ApplicationStartup, DefaultConstructedApplicationDoesNotStart) {
    Application app;
    Callbacks   callbacks;
    callbacks.attach(app);

    auto started = app.start();

    ASSERT_TRUE(started.is_err());
    EXPECT_EQ(started.error().m_code, InitErrorCode::NOT_INITIALIZED);
    expect_not_started(app, callbacks);
}

TEST_F(ApplicationStartup, UnsupportedBackendIsPropagatedAndTheMainLoopIsNotEntered) {
    Instance& instance = shared_instance();
#if defined(__linux__)
    const auto windows_before = instance.m_system_manager.m_windows.size();
#endif
    (void)instance;

    for (GRAPHICS_API api : apis_without_renderer()) {
        SCOPED_TRACE(to_string(api));
        Application app(Application::Desc{
            .title = "Unsupported backend",
            .size = v2u32::create(64, 64),
            .position = v2u32::create(0, 0),
            .api = api,
        });
        Callbacks callbacks;
        callbacks.attach(app);

        ASSERT_TRUE(app.m_init_result.is_err());
        EXPECT_EQ(app.m_init_result.error().m_code, InitErrorCode::BACKEND_UNSUPPORTED);

        auto started = app.start();

        ASSERT_TRUE(started.is_err());
        EXPECT_EQ(started.error().m_code, InitErrorCode::BACKEND_UNSUPPORTED);
        EXPECT_EQ(started.error().m_api, api);
        expect_not_started(app, callbacks);

        // Starting again does not get any further.
        EXPECT_TRUE(app.start().is_err());
        expect_not_started(app, callbacks);
    }

#if defined(__linux__)
    // The backend is checked before a window is opened.
    EXPECT_EQ(instance.m_system_manager.m_windows.size(), windows_before);
#endif
}

#if defined(__linux__)
TEST_F(ApplicationStartup, WindowCreationFailureIsPropagated) {
    const std::vector<GRAPHICS_API> backends = compiled_backends();
    if (backends.empty()) { GTEST_SKIP() << "No backend is compiled in"; }
    (void)shared_instance();

    // Without DISPLAY the X11 window cannot be created.
    const char*       display = std::getenv("DISPLAY");
    const std::string saved_display = display == nullptr ? "" : display;
    ASSERT_EQ(unsetenv("DISPLAY"), 0);

    {
        Application app(Application::Desc{
            .title = "No display",
            .size = v2u32::create(64, 64),
            .position = v2u32::create(0, 0),
            .api = backends.front(),
        });
        Callbacks callbacks;
        callbacks.attach(app);

        auto started = app.start();

        EXPECT_TRUE(started.is_err());
        if (started.is_err()) {
            EXPECT_EQ(started.error().m_code, InitErrorCode::WINDOW_CREATION_FAILED);
            EXPECT_EQ(started.error().m_api, backends.front());
            EXPECT_NE(started.error().m_message.find("No display"), std::string::npos)
                << started.error().m_message;
        }
        expect_not_started(app, callbacks);
    }

    if (display != nullptr) { ASSERT_EQ(setenv("DISPLAY", saved_display.c_str(), 1), 0); }
}
#endif
