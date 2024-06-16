#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #include "JadeFrame/platform/windows/windows_input_manager.h"
    #include "JadeFrame/platform/windows/windows_system_manager.h"
    #include "JadeFrame/platform/windows/windows_window.h"
#elif defined(__linux__)
    #include "JadeFrame/platform/linux/linux_input_manager.h"
    #include "JadeFrame/platform/linux/linux_system_manager.h"
    #include "JadeFrame/platform/linux/linux_window.h"
#endif
#include "JadeFrame/utils/logger.h"
#include "JadeFrame/math/vec.h"
#include "graphics/camera.h"
#include "graphics/graphics_shared.h"
#include "JadeFrame/graphics/mesh.h"
#include "gui.h"

#include <deque>
#include <map>
#include <memory>

namespace JadeFrame {

/*
        The struct JadeFrame should act more or less like the ultimate global
   scope of JadeFrame. In here goes everything which would not make sense to be
   part of the application itself.

        It also allows an abtraction, which may allow to have several
   applications at the same time in a JadeFrame context.
*/
auto control_camera(Camera* self) -> void;
#ifdef _WIN32
using SystemManager = win32::SystemManager;
using InputManager = win32::InputManager;
#elif __linux__
using SystemManager = Linux_SystemManager;
using InputManager = Linux_InputManager;
#endif

class BaseApp;
class IRenderer;

class BaseApp {
public:
    struct Desc {
        std::string  title;
        v2u32        size;
        v2u32        position = {0, 0};
        GRAPHICS_API api;
    };

    BaseApp() = default;
    explicit BaseApp(const Desc& desc);
    BaseApp(const BaseApp&) = delete;
    auto operator=(const BaseApp&) -> BaseApp& = delete;
    BaseApp(BaseApp&&) = delete;
    auto operator=(BaseApp&&) -> BaseApp& = delete;
    virtual ~BaseApp() = default;

    virtual auto on_init() -> void = 0;
    virtual auto on_update() -> void = 0;
    virtual auto on_draw() -> void = 0;

    auto start() -> void;
    // protected:
    auto poll_events() -> void;

public:
    bool m_is_running = true;

    // Window stuff
    using WindowID = i32;
    std::map<WindowID, Window*> m_windows;
    Window*                     m_current_window_p = nullptr;

    RenderSystem m_render_system;

    Camera m_camera;

    GUI m_gui;
    u64 m_tick = 0;
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

    auto        run() -> void;
    static auto get_singleton() -> Instance*;

    template<typename T>
    auto request_app(BaseApp::Desc desc) -> T* {
        // For now only one app is allowed
        assert(m_apps.empty());
        m_apps.emplace_back(new T(desc));
        return (T*)m_apps.back();
    }

    auto register_app(BaseApp* app) -> void {
        // For now only one app is allowed
        assert(m_apps.empty());
        m_apps.emplace_back(app);
    }

public:
    CompilerInfo m_compiler_info;
    std::string  m_platform_info;
    std::string  m_architecture_info;
    u32          m_cpp_version;

    SystemManager m_system_manager;
    InputManager  m_input_manager;

    std::deque<BaseApp*>
        m_apps; // TODO: Consider whether there should be support for multiple apps
    // std::deque<BaseApp> m_apps;

    BaseApp* m_current_app_p = nullptr;

    static Instance* m_singleton;
};

} // namespace JadeFrame
