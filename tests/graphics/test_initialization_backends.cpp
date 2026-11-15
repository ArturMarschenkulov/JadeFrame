// Successful initialization and shutdown of every backend that is compiled in and usable
// on this machine, through the render system and through the application. These tests
// need a display and a GPU (or a software implementation) and are skipped otherwise.

#include <gtest/gtest.h>

#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include "JadeFrame/base_app.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/graphics/mesh.h"

using namespace JadeFrame;

namespace {

// `Instance` is a process-wide singleton that cannot be created twice.
auto shared_instance() -> Instance& {
    static auto* instance = new Instance();
    return *instance;
}

auto shared_window(GRAPHICS_API api) -> Window* {
    static std::map<GRAPHICS_API, Window*> windows;
    auto                                   it = windows.find(api);
    if (it == windows.end()) {
        Window* window = shared_instance().m_system_manager.request_window(Window::Desc{
            .title = std::string("Initialization - ") + to_string(api),
            .size = v2u32::create(160, 120),
            .position = v2u32::create(0, 0),
        });
        it = windows.emplace(api, window).first;
    }
    return it->second;
}

class BackendInitialization : public testing::TestWithParam<GRAPHICS_API> {
protected:
    auto SetUp() -> void override {
        const GRAPHICS_API api = GetParam();

        bool available = false;
        for (GRAPHICS_API candidate : RenderSystem::list_available_graphics_apis()) {
            available = available || candidate == api;
        }
        if (!available) { GTEST_SKIP() << to_string(api) << " is not available"; }
        if (std::getenv("DISPLAY") == nullptr) { GTEST_SKIP() << "No display"; }
    }
};

TEST_P(BackendInitialization, RenderSystemInitializesRendersAndShutsDownRepeatedly) {
    const GRAPHICS_API api = GetParam();
    Window*            window = nullptr;
    try {
        window = shared_window(api);
    } catch (const std::exception& e) { GTEST_SKIP() << "No window: " << e.what(); }

    // More than once on the same window: a shutdown has to release everything the
    // next initialization needs.
    for (i32 round = 0; round < 3; ++round) {
        SCOPED_TRACE(round);
        RenderSystem system;

        auto initialized = system.init(api, window);
        if (initialized.is_err() && round == 0) {
            GTEST_SKIP() << "Cannot create a " << to_string(api)
                         << " renderer: " << to_string(initialized.error());
        }
        ASSERT_TRUE(initialized.is_ok()) << to_string(initialized.error());
        ASSERT_TRUE(system.is_initialized());
        ASSERT_NE(system.m_renderer, nullptr);
        EXPECT_EQ(system.m_api, api);

        const Camera camera = Camera::orthographic(-1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F);
        for (i32 frame = 0; frame < 2; ++frame) {
            system.m_renderer->clear_background();
            system.m_renderer->render(camera);
            system.m_renderer->present();
        }

        if (round == 1) {
            // Explicit shutdown, the destructor then has nothing left to do.
            system.shutdown();
            EXPECT_FALSE(system.is_initialized());
            EXPECT_EQ(system.m_renderer, nullptr);
            EXPECT_EQ(system.m_api, GRAPHICS_API::UNDEFINED);
        }
    }
}

TEST_P(BackendInitialization, ReinitializingAnInitializedRenderSystemWorks) {
    const GRAPHICS_API api = GetParam();
    Window*            window = nullptr;
    try {
        window = shared_window(api);
    } catch (const std::exception& e) { GTEST_SKIP() << "No window: " << e.what(); }

    RenderSystem system;
    auto         first = system.init(api, window);
    if (first.is_err()) {
        GTEST_SKIP() << "Cannot create a " << to_string(api)
                     << " renderer: " << to_string(first.error());
    }

    auto second = system.init(api, window);

    ASSERT_TRUE(second.is_ok()) << to_string(second.error());
    EXPECT_TRUE(system.is_initialized());
    EXPECT_EQ(system.m_api, api);
}

TEST_P(BackendInitialization, ApplicationStartsRunsItsMainLoopAndShutsDown) {
    const GRAPHICS_API api = GetParam();
    (void)shared_instance();

    i32 inits = 0;
    i32 updates = 0;
    i32 draws = 0;
    i32 rendered_frames = 0;
    {
        Application app(Application::Desc{
            .title = "Application startup",
            .size = v2u32::create(160, 120),
            .position = v2u32::create(0, 0),
            .api = api,
        });
        if (app.m_init_result.is_err()) {
            GTEST_SKIP() << "Cannot start a " << to_string(api)
                         << " application: " << to_string(app.m_init_result.error());
        }
        ASSERT_TRUE(app.m_render_system.is_initialized());
        EXPECT_EQ(app.m_render_system.m_api, api);
        EXPECT_EQ(app.m_windows.size(), 1U);

        const u64 frames = 3;
        app.m_on_init_fn = [&] {
            ++inits;
            // The renderer is usable from the first callback on.
            app.m_render_system.m_renderer->set_clear_color(RGBAColor::solid_black());
        };
        app.m_on_update_fn = [&] {
            ++updates;
            if (app.m_tick + 1 >= frames) { app.m_is_running = false; }
        };
        app.m_on_draw_fn = [&] { ++draws; };
        app.m_on_frame_rendered_fn = [&] { ++rendered_frames; };

        auto started = app.start();

        ASSERT_TRUE(started.is_ok()) << to_string(started.error());
        EXPECT_EQ(app.m_tick, frames);
        // The application is destroyed here: GUI, registered resources, renderer.
    }
    EXPECT_EQ(inits, 1);
    EXPECT_EQ(updates, 3);
    EXPECT_EQ(draws, 3);
    EXPECT_EQ(rendered_frames, 3);
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
    BackendInitialization,
    testing::ValuesIn(compiled_backends()),
    [](const testing::TestParamInfo<GRAPHICS_API>& info) {
        return std::string(to_string(info.param));
    }
);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BackendInitialization);

} // namespace
