#include "pch.h"
#include "base_app.h"
#include "graphics/graphics_shared.h"

#include "gui.h"

#include "JadeFrame/utils/option.h"
#include "JadeFrame/utils/result.h"
#include "../extern/result.hpp"

#include "JadeFrame/math/vec.h"

#include "JadeFrame/utils/utils.h"
#include "JadeFrame/math/math.h"
#include "JadeFrame/ptr/ptr.h"

#include <utility>

namespace JadeFrame {


auto control_camera(Camera* self) -> void {
    const f32           velocity = 0.1f;
    const InputManager& i = Instance::get_singleton()->m_input_manager;
    if (self->m_mode == Camera::MODE::PERSPECTIVE) {
        if (i.is_key_down(KEY::E)) self->m_position += self->m_up * velocity;
        if (i.is_key_down(KEY::Q)) self->m_position -= self->m_up * velocity;

        if (i.is_key_down(KEY::A)) self->m_position -= self->m_right * velocity;
        if (i.is_key_down(KEY::D)) self->m_position += self->m_right * velocity;

        if (i.is_key_down(KEY::S)) self->m_position -= self->m_forward * velocity;
        if (i.is_key_down(KEY::W)) self->m_position += self->m_forward * velocity;

        auto sensitivity = 10;
        if (i.is_key_down(KEY::LEFT)) self->m_pitch += velocity * sensitivity;
        if (i.is_key_down(KEY::RIGHT)) self->m_pitch -= velocity * sensitivity;
        if (i.is_key_down(KEY::UP)) self->m_yaw += velocity * sensitivity;
        if (i.is_key_down(KEY::DOWN)) self->m_yaw -= velocity * sensitivity;


        // if (m_pitch > 89.0f)
        //	m_pitch = 89.0f;
        // if (m_pitch < -89.0f)
        //	m_pitch = -89.0f;

        v3 front;
        front.x = cos(to_radians(self->m_yaw)) * cos(to_radians(self->m_pitch));
        front.y = sin(to_radians(self->m_pitch));
        front.z = sin(to_radians(self->m_yaw)) * cos(to_radians(self->m_pitch));
        self->m_forward = front.get_normal();

        self->m_right = self->m_forward.cross(self->m_world_up).get_normal();
        self->m_up = self->m_right.cross(self->m_forward).get_normal();
    } else if (self->m_mode == Camera::MODE::ORTHOGRAPHIC) {
        if (i.is_key_down(KEY::E)) self->m_position += self->m_up * velocity;
        if (i.is_key_down(KEY::Q)) self->m_position -= self->m_up * velocity;

        if (i.is_key_down(KEY::A)) self->m_position -= self->m_right * velocity;
        if (i.is_key_down(KEY::D)) self->m_position += self->m_right * velocity;

        if (i.is_key_down(KEY::S)) self->m_position -= self->m_forward * velocity;
        if (i.is_key_down(KEY::W)) self->m_position += self->m_forward * velocity;

        auto sensitivity = 10;
        if (i.is_key_down(KEY::LEFT)) self->m_pitch += velocity * sensitivity;
        if (i.is_key_down(KEY::RIGHT)) self->m_pitch -= velocity * sensitivity;
        if (i.is_key_down(KEY::UP)) self->m_yaw += velocity * sensitivity;
        if (i.is_key_down(KEY::DOWN)) self->m_yaw -= velocity * sensitivity;
    }
}

//**************************************************************
// JadeFrame
//**************************************************************
Instance* Instance::m_singleton = nullptr;

auto Instance::get_singleton() -> Instance* { return m_singleton; }




consteval auto get_cpp_version() -> u32 {

#if defined(_MSC_VER)
    constexpr u32 version_raw = _MSVC_LANG;
#else
    constexpr u32         version_raw = __cplusplus;
#endif
    if constexpr (version_raw == 0) {
        return version_raw;
    } else if constexpr (version_raw >= 202000L) {
        return 20;
    } else if constexpr (version_raw >= 201700L) {
        return 17;
    } else if constexpr (version_raw >= 201400L) {
        return 14;
    } else if constexpr (version_raw >= 202000L) {
        return 11;
    } else if constexpr (version_raw >= 199800L) {
        return 98;
    } else {
        return version_raw;
    }
}

consteval auto get_compiler_info() -> CompilerInfo {
#if defined(__clang__)
    constexpr const char* name = "CLANG";
    constexpr u32         major = __clang_major__;
    constexpr u32         minor = __clang_minor__;
    constexpr u32         patch = __clang_patchlevel__;
#elif defined(__GNUC__)
    constexpr const char* name = "GCC";
    constexpr u32         major = __GNUC__;
    constexpr u32         minor = __GNUC_MINOR__;
    constexpr u32         patch = __GNUC_PATCHLEVEL__;
#elif defined(_MSC_VER)
    constexpr const char* name = "MSVC";
    constexpr u32         major = _MSC_VER / 100;
    constexpr u32         minor = _MSC_VER - (_MSC_VER / 100 * 100);
    constexpr u32         patch = _MSC_FULL_VER - _MSC_VER * 100000;

#elif defined(__INTEL_COMPILER)
    constexpr const char* name = "ICC";
    constexpr u32         major = __INTEL_COMPILER / 100;
    constexpr u32         minor = __INTEL_COMPILER % 100;
    constexpr u32         patch = __INTEL_COMPILER % 10;
#elif defined(__MINGW64__)
    constexpr const char* name = "MINGW64";
    constexpr u32         major = __MINGW64_VERSION_MAJOR;
    constexpr u32         minor = __MINGW64_VERSION_MINOR;
    constexpr u32         patch = 0;
#elif defined(__MINGW32__)
    constexpr const char* name = "MINGW32";
    constexpr u32         major = __MINGW32_MAJOR_VERSION;
    constexpr u32         minor = __MINGW32_MAJOR_VERSION;
    constexpr u32         patch = 0;
#elif defined(CYGWIN)
    constexpr const char* name = "CYGWIN";
    constexpr u32         major = CYGWIN_VERSION_DLL_MAJOR;
    constexpr u32         minor = CYGWIN_VERSION_DLL_MINOR;
    constexpr u32         patch = 0;
#else
    constexpr const char* name = "UNKNOWN";
    constexpr u32         major = 0;
    constexpr u32         minor = 0;
    constexpr u32         patch = 0;
#endif


    constexpr CompilerInfo info = {
        name, {major, minor, patch}
    };
    return info;
} // namespace JadeFrame
consteval auto get_plattform_info() -> const char* {
    const char* name = "UNKNOWN";
#if defined(_WIN32)
    name = "WIN32";
#elif defined(__linux__) && !defined(__ANDROID__)
    name = "LINUX";
#elif defined(__ANDROID__)
    name = "ANDROID";
#elif defined(__APPLE__)
    name = "APPLE";
#else
    name = "UNKNOWN";
#endif
    return name;
}
consteval auto get_architecture_info() -> const char* {
    const char* name = "UNKNOWN";
#if defined(__x86_64__) || defined(_M_X64)
    name = "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    name = "i386";
#elif defined(__arm__) || defined(_M_ARM)
    name = "ARM";
#elif defined(__aarch64__) || defined(_M_ARM64)
    name = "ARM64";
#elif
    name = "UNKNOWN";
#endif
    return name;
}


auto test_modules() -> void {
    option::test();
    result::test();
}


auto to_double(const char* str) noexcept -> Result<double, std::errc> {
    auto* last_entry = static_cast<char*>(nullptr);

    errno = 0;
    const auto result = std::strtod(str, &last_entry);

    if (errno != 0) {
        // Returns an error value
        return Failure(static_cast<std::errc>(errno));
    }
    // Returns a value
    return result;
}

Instance::Instance() {
    Logger::init();
    Logger::info("Logger initialized");
    Logger::info("JadeFrame is starting...");
    if (m_singleton != nullptr) {
        Logger::err("Instance already exists");
        assert(false);
        return;
    }
    m_singleton = this;

    const CompilerInfo& ci = m_compiler_info = get_compiler_info();
    const std::string&  pi = m_platform_info = get_plattform_info();
    const std::string&  ai = m_architecture_info = get_architecture_info();
    const u32&          li = m_cpp_version = get_cpp_version();

    Logger::info("Detected Plattform is '{}'", pi);
    Logger::info("Detected Architecture is '{}'", ai);
    Logger::info(
        "Detected Compiler is '{}' with version '{}.{}.{}'", ci.name, ci.version.major, ci.version.minor,
        ci.version.patch);
    Logger::info("Detected C++ Version is '{}'", li);

    m_system_manager.initialize();
    // m_input_manager.initialize();

    m_system_manager.log();
}
auto Instance::run() -> void {
    Logger::info("App Running");
    m_current_app_p = m_apps[0];
    m_apps.back()->start();
}
//**************************************************************
//~JadeFrame
//**************************************************************

//**************************************************************
// BaseApp
//**************************************************************

BaseApp::BaseApp(const Desc& desc) {
    Logger::info("Creating Window....");
    IWindow::Desc win_desc = {
        .title = desc.title,
        .size = desc.size,
        .position = desc.position,
    };
    auto i = Instance::get_singleton();
    m_windows[0] = i->m_system_manager.request_window(win_desc);
    m_current_window_p = m_windows[0];

    Logger::info("Creating Renderer");
    m_render_system.init(desc.api, m_windows[0]);
    // m_render_system = m_system_manager.request_render_system(api, m_windows[0]);


    const std::string& title = m_current_window_p->get_title();

    std::string new_title = title + " - " + to_string(desc.api);

    m_current_window_p->set_title(new_title);
    // m_gui.init(m_current_window_p, api);
}
inline auto to_string(const Matrix4x4& m) -> std::string {
    std::string result;
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = col + 1; row < 4; row++) {
            auto s = std::to_string(m[col][row]);
            result += s;
            result += ", ";
        }
    }
    return result;
}
auto BaseApp::start() -> void {
    // Before `this->on_init();` come all the default stuff
    // The client can later override those in `this->on_init();`
    m_camera.othographic_mode(0, m_windows[0]->get_size().x, m_windows[0]->get_size().y, 0, -1, 1);
    SystemManager& platform = Instance::get_singleton()->m_system_manager;
    platform.set_target_FPS(60);
    this->on_init();

    IRenderer* renderer = m_render_system.m_renderer;
    while (m_is_running) {
        const f64 delta_time = platform.calc_elapsed();
        this->on_update();

        if (m_current_window_p->get_window_state() != IWindow::WINDOW_STATE::MINIMIZED) {
            renderer->clear_background();
            // m_gui.new_frame();

            this->on_draw();
            const Matrix4x4& view_projection = m_camera.get_view_projection();
            // control_camera(&m_camera);
            renderer->render(view_projection);

            // m_gui.render();

            renderer->present();
            m_tick += 1;
        }
        this->poll_events();
        platform.frame_control(delta_time);
    }
}
auto BaseApp::poll_events() -> void {
    Instance::get_singleton()->m_input_manager.handle_input();
    m_windows[0]->handle_events(m_is_running);
}
//**************************************************************
//~BaseApp
//**************************************************************


} // namespace
  // JadeFrame