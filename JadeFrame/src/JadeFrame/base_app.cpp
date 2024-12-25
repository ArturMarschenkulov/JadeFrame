#include "pch.h"
#include "base_app.h"
#include "graphics/graphics_shared.h"

#include "gui.h"

#include "JadeFrame/math/vec.h"

#include "JadeFrame/math/math.h"
#include "JadeFrame/ptr/ptr.h"

namespace JadeFrame {

auto control_camera(Camera* self, const InputState& i) -> void {
    const f32 velocity = 0.1F;
    if (self->m_mode == Camera::MODE::PERSPECTIVE) {
        if (i.is_key_down(KEY::E)) { self->m_position += self->m_up * velocity; }
        if (i.is_key_down(KEY::Q)) { self->m_position -= self->m_up * velocity; }

        if (i.is_key_down(KEY::A)) { self->m_position -= self->m_right * velocity; }
        if (i.is_key_down(KEY::D)) { self->m_position += self->m_right * velocity; }

        if (i.is_key_down(KEY::S)) { self->m_position -= self->m_forward * velocity; }
        if (i.is_key_down(KEY::W)) { self->m_position += self->m_forward * velocity; }

        const f32 sensitivity = 10;
        if (i.is_key_down(KEY::LEFT)) { self->m_pitch += velocity * sensitivity; }
        if (i.is_key_down(KEY::RIGHT)) { self->m_pitch -= velocity * sensitivity; }
        if (i.is_key_down(KEY::UP)) { self->m_yaw += velocity * sensitivity; }
        if (i.is_key_down(KEY::DOWN)) { self->m_yaw -= velocity * sensitivity; }

        // if (m_pitch > 89.0f)
        //	m_pitch = 89.0f;
        // if (m_pitch < -89.0f)
        //	m_pitch = -89.0f;

        v3 front;
        front.x = std::cos(to_radians(self->m_yaw)) * std::cos(to_radians(self->m_pitch));
        front.y = std::sin(to_radians(self->m_pitch));
        front.z = std::sin(to_radians(self->m_yaw)) * std::cos(to_radians(self->m_pitch));
        self->m_forward = front.normalize();

        self->m_right = self->m_forward.cross(self->m_world_up).normalize();
        self->m_up = self->m_right.cross(self->m_forward).normalize();
    } else if (self->m_mode == Camera::MODE::ORTHOGRAPHIC) {
        if (i.is_key_down(KEY::E)) { self->m_position += self->m_up * velocity; }
        if (i.is_key_down(KEY::Q)) { self->m_position -= self->m_up * velocity; }

        if (i.is_key_down(KEY::A)) { self->m_position -= self->m_right * velocity; }
        if (i.is_key_down(KEY::D)) { self->m_position += self->m_right * velocity; }

        if (i.is_key_down(KEY::S)) { self->m_position -= self->m_forward * velocity; }
        if (i.is_key_down(KEY::W)) { self->m_position += self->m_forward * velocity; }

        const f32 sensitivity = 10;
        if (i.is_key_down(KEY::LEFT)) { self->m_pitch += velocity * sensitivity; }
        if (i.is_key_down(KEY::RIGHT)) { self->m_pitch -= velocity * sensitivity; }
        if (i.is_key_down(KEY::UP)) { self->m_yaw += velocity * sensitivity; }
        if (i.is_key_down(KEY::DOWN)) { self->m_yaw -= velocity * sensitivity; }
    }
}

//**************************************************************
// JadeFrame
//**************************************************************
Instance* Instance::m_singleton = nullptr;

auto Instance::get_singleton() -> Instance* { return m_singleton; }

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
        "Detected Compiler is '{}' with version '{}.{}.{}'",
        ci.name,
        ci.version.major,
        ci.version.minor,
        ci.version.patch
    );
    Logger::info("Detected C++ Version is '{}'", li);

    m_system_manager.initialize();

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
    Window::Desc win_desc = {
        .title = desc.title,
        .size = desc.size,
        .position = desc.position,
    };
    auto* i = Instance::get_singleton();
    m_windows[0] = i->m_system_manager.request_window(win_desc);
    m_current_window_p = m_windows[0];

    auto apis = RenderSystem::list_available_graphics_apis();
    for (auto& api : apis) { Logger::info("API: '{}'", to_string(api)); }
    Logger::info("Creating Renderer with API '{}'", to_string(desc.api));
    m_render_system.init(desc.api, m_windows[0]);
    // m_render_system = m_system_manager.request_render_system(api, m_windows[0]);

    const std::string& title = m_current_window_p->get_title();
    std::string        new_title = title + " - " + to_string(desc.api);
    m_current_window_p->set_title(new_title);
    // m_gui.init(m_current_window_p, api);
}

inline auto to_string(const mat4x4& m) -> std::string {
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
    auto& curr_win = m_windows[0];
    m_camera =
        Camera::orthographic(0, curr_win->get_size().x, curr_win->get_size().y, 0, -1, 1);
    SystemManager& platform = Instance::get_singleton()->m_system_manager;
    platform.set_target_FPS(60);
    this->on_init();

    IRenderer* renderer = m_render_system.m_renderer;
    while (m_is_running) {
        const f64 delta_time = platform.calc_elapsed();
        this->on_update();
        control_camera(&m_camera, m_windows[0]->m_input_state);

        if (m_current_window_p->get_window_state() != Window::WINDOW_STATE::MINIMIZED) {
            renderer->clear_background();
            // m_gui.new_frame();

            this->on_draw();
            const mat4x4& view_projection = m_camera.get_view_projection();
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
    for (auto& [id, window] : m_windows) { window->handle_events(m_is_running); }
}

//**************************************************************
//~BaseApp
//**************************************************************

} // namespace
  // JadeFrame
