
#include "base_app.h"
#include "graphics/graphics_shared.h"

#include <imgui/imgui.h>
#include "gui.h"

namespace JadeFrame {

auto control_camera(Camera* self, const InputState& i) -> void {
    const f32 velocity = 1.0F;
    const f32 sensitivity = 1;

    Camera::Orientation& orient = self->m_orientation;

    if (i.is_key_down(KEY::E)) { self->m_position += orient.m_up * velocity; }
    if (i.is_key_down(KEY::Q)) { self->m_position -= orient.m_up * velocity; }

    if (i.is_key_down(KEY::A)) { self->m_position -= orient.m_right * velocity; }
    if (i.is_key_down(KEY::D)) { self->m_position += orient.m_right * velocity; }

    if (i.is_key_down(KEY::S)) { self->m_position -= orient.m_forward * velocity; }
    if (i.is_key_down(KEY::W)) { self->m_position += orient.m_forward * velocity; }
    {
        auto [pitch, yaw] = orient.get_pitch_yaw();
        if (i.is_key_down(KEY::LEFT)) { yaw += velocity * sensitivity; }
        if (i.is_key_down(KEY::RIGHT)) { yaw -= velocity * sensitivity; }
        if (i.is_key_down(KEY::UP)) { pitch -= velocity * sensitivity; }
        if (i.is_key_down(KEY::DOWN)) { pitch += velocity * sensitivity; }

        pitch = std::clamp(pitch, -89.0f, 89.0f);
        orient.set_pitch_yaw(pitch, yaw);
    }

    // ImGui::Text(
    //     "Position: %f, %f, %f", self->m_position.x, self->m_position.y,
    //     self->m_position.z
    // );
    // ImGui::Text(
    //     "Forward: %f, %f, %f", orient.m_forward.x, orient.m_forward.y,
    //     orient.m_forward.z
    // );
    // ImGui::Text(
    //     "Right: %f, %f, %f", orient.m_right.x, orient.m_right.y, orient.m_right.z
    // );
    // ImGui::Text("Up: %f, %f, %f", orient.m_up.x, orient.m_up.y, orient.m_up.z);
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
    Instance* i = Instance::get_singleton();
    m_windows[0] = i->m_system_manager.request_window(win_desc);
    m_current_window_p = m_windows[0];

    std::vector<GRAPHICS_API> apis = RenderSystem::list_available_graphics_apis();
    for (GRAPHICS_API& api : apis) { Logger::info("API: '{}'", to_string(api)); }
    Logger::info("Creating Renderer with API '{}'", to_string(desc.api));
    m_render_system.init(desc.api, m_windows[0]);
    // m_render_system = m_system_manager.request_render_system(api, m_windows[0]);

    const std::string& title = m_current_window_p->get_title();
    std::string        new_title = title + " - " + to_string(desc.api);
    m_current_window_p->set_title(new_title);
    m_gui.init(m_current_window_p, desc.api);
}

auto BaseApp::start() -> void {
    // Before `this->on_init();` come all the default stuff
    // The client can later override those in `this->on_init();`
    Window* curr_win = m_windows[0];
    m_camera =
        Camera::orthographic(0, curr_win->get_size().x, curr_win->get_size().y, 0, -1, 1);
    SystemManager& platform = Instance::get_singleton()->m_system_manager;
    platform.set_target_FPS(60);
    this->on_init();

    IRenderer* renderer = m_render_system.m_renderer;
    while (m_is_running) {
        const f64 delta_time = platform.calc_elapsed();
        this->on_update();

        // if (m_current_window_p->get_window_state() != Window::WINDOW_STATE::MINIMIZED)
        // {
        renderer->clear_background();

        if (m_gui.m_is_initialized) { m_gui.new_frame(); }
        control_camera(&m_camera, m_windows[0]->m_input_state);

        this->on_draw();
        const mat4x4& view_projection = m_camera.get_view_projection();
        // control_camera(&m_camera);
        renderer->render(view_projection);
        if (m_gui.m_is_initialized) { m_gui.render(); }

        renderer->present();
        m_tick += 1;
        //}
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
