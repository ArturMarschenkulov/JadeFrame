#pragma once

#include <deque>
#include <map>
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
    #include "JadeFrame/platform/windows/windows_input_manager.h"
    #include "JadeFrame/platform/windows/windows_system_manager.h"
    #include "JadeFrame/platform/windows/windows_window.h"
#elif defined(__linux__)
    #include "JadeFrame/platform/linux/linux_system_manager.h"
#endif
#include "JadeFrame/math/vec.h"
#include "graphics/camera.h"
#include "graphics/graphics_shared.h"
#include "gui.h"

namespace JadeFrame {

/*
        The struct JadeFrame should act more or less like the ultimate global
   scope of JadeFrame. In here goes everything which would not make sense to be
   part of the application itself.

        It also allows an abtraction, which may allow to have several
   applications at the same time in a JadeFrame context.
*/
auto control_camera(Camera& self, const InputState& i, f64 delta_seconds) -> void;
#ifdef _WIN32
using SystemManager = win32::SystemManager;
#elif __linux__
using SystemManager = Linux_SystemManager;
#endif

class IRenderer;

class Application {
public:
    struct Desc {
        std::string  title;
        v2u32        size;
        v2u32        position = v2u32::create(0, 0);
        GRAPHICS_API api;
    };

    Application() = default;
    /// Creates the window, the renderer and the GUI. A constructor cannot return an
    /// error, so the outcome is kept in `m_init_result` and returned by `start()`.
    ///
    /// The backend is exactly `desc.api`. If the window or the renderer cannot be
    /// created the application stays uninitialized: it has no renderer
    /// (`m_render_system.m_renderer` is null), no GUI and no entry in `m_windows`.
    /// Destroying it is safe in either state.
    explicit Application(const Desc& desc);
    Application(const Application&) = delete;
    auto operator=(const Application&) -> Application& = delete;
    Application(Application&&) = delete;
    auto operator=(Application&&) -> Application& = delete;
    /// Safe after a failed, partial or successful initialization. Members are
    /// destroyed in reverse order: the GUI first, the render system (which releases
    /// the registered resources and then the renderer) after it.
    ~Application();

public:
    std::function<void()> m_on_init_fn = []() -> void {};
    std::function<void()> m_on_update_fn = []() -> void {};
    std::function<void()> m_on_draw_fn = []() -> void {};
    /// Called once per frame after the frame (including the GUI) was rendered and
    /// before it is presented. This is the point at which
    /// `IRenderer::take_screenshot` can be used.
    std::function<void()> m_on_frame_rendered_fn = []() -> void {};

    /// Runs the application until `m_is_running` is cleared.
    ///
    /// If initialization failed this returns that error (`m_init_result`) right away:
    /// no callback is invoked, the main loop is not entered and the renderer is not
    /// touched. A default constructed application fails with `NOT_INITIALIZED`.
    [[nodiscard]] auto start() -> Result<void, InitError>;
    auto               poll_events() -> void;

public:
    /// Outcome of the initialization done by the constructor.
    Result<void, InitError> m_init_result = err<void, InitError>(InitError{
        InitErrorCode::NOT_INITIALIZED,
        GRAPHICS_API::UNDEFINED,
        "the application was not created from an Application::Desc",
    });

    bool m_is_running = true;

    // Window stuff
    using WindowID = i32;
    std::map<WindowID, Window*> m_windows;
    Window*                     m_current_window_p = nullptr;

    RenderSystem m_render_system;

    Camera m_camera;

    GUI m_gui;
    u64 m_tick = 0;

private:
    // The window outlives the application, so the callback has to be unregistered.
    u64 m_resize_callback_id = 0;

    auto init(const Desc& desc) -> Result<void, InitError>;
};

/*
    The instance is JadeFrames global scope, so to say. It can also be regarded as the
   first place to put stuff if one does not know where to put it.
*/

class Instance {
public:
    Instance();
    ~Instance() = default;

    Instance(const Instance&) = delete;
    auto operator=(const Instance&) -> Instance& = delete;

    Instance(Instance&&) = delete;
    auto operator=(Instance&&) -> Instance& = delete;

    static auto get_singleton() -> Instance*;

    auto request_application(Application::Desc desc) -> Application* {
        // For now only one app is allowed
        assert(m_applications.empty());
        m_applications.emplace_back(std::make_unique<Application>(desc));
        m_current_app_p = m_applications.back().get();
        return m_current_app_p;
    }

public:
    CompilerInfo m_compiler_info;
    std::string  m_platform_info;
    std::string  m_architecture_info;
    u32          m_cpp_version;

    SystemManager                             m_system_manager;
    std::deque<std::unique_ptr<Application>> m_applications; // TODO: Consider whether
                                                             // there should be support
                                                             // for multiple apps

    // TODO(artur): This should be removed.
    Application* m_current_app_p = nullptr;

    static Instance* m_singleton;
};

} // namespace JadeFrame
