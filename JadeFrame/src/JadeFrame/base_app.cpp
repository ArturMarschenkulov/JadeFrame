
#include "base_app.h"
#include "JadeFrame/platform/platform_shared.h"
#include "graphics/graphics_shared.h"

#include "gui.h"

namespace JadeFrame {

auto control_camera(Camera& self, const InputState& i, f64 delta_seconds_) -> void {
    f32       delta_seconds = delta_seconds_;
    const f32 velocity = 1.0F;
    const f32 sensitivity = 1;

    Camera::Orientation& orient = self.m_orientation;

    if (i.is_key_down(KEY::E)) {
        self.m_position += orient.m_up * velocity * delta_seconds;
    }
    if (i.is_key_down(KEY::Q)) {
        self.m_position -= orient.m_up * velocity * delta_seconds;
    }

    if (i.is_key_down(KEY::A)) {
        self.m_position -= orient.m_right * velocity * delta_seconds;
    }
    if (i.is_key_down(KEY::D)) {
        self.m_position += orient.m_right * velocity * delta_seconds;
    }

    if (i.is_key_down(KEY::S)) {
        self.m_position -= orient.m_forward * velocity * delta_seconds;
    }
    if (i.is_key_down(KEY::W)) {
        self.m_position += orient.m_forward * velocity * delta_seconds;
    }
    {
        auto [pitch, yaw] = orient.get_pitch_yaw();
        if (i.is_key_down(KEY::LEFT)) { yaw += velocity * sensitivity * delta_seconds; }
        if (i.is_key_down(KEY::RIGHT)) { yaw -= velocity * sensitivity * delta_seconds; }
        if (i.is_key_down(KEY::UP)) { pitch -= velocity * sensitivity * delta_seconds; }
        if (i.is_key_down(KEY::DOWN)) { pitch += velocity * sensitivity * delta_seconds; }

        pitch = std::clamp(pitch, -89.0F, 89.0F);
        orient.set_pitch_yaw(pitch, yaw);
    }

    // ImGui::Text(
    //     "Position: %f, %f, %f", self.m_position.x, self.m_position.y,
    //     self.m_position.z
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

Instance::Instance()
    : m_compiler_info(get_compiler_info())
    , m_platform_info(get_platform_info())
    , m_architecture_info(get_architecture_info())
    , m_cpp_version(get_cpp_version()) {
    Logger::init();
    Logger::info("Logger initialized");
    const CompilerInfo& ci = m_compiler_info;
    const std::string&  pi = m_platform_info;
    const std::string&  ai = m_architecture_info;
    const u32&          li = m_cpp_version;

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

    Logger::info("JadeFrame is starting...");
    if (m_singleton != nullptr) {
        Logger::err("Instance already exists");
        assert(false);
        return;
    }
    m_singleton = this;

    m_system_manager.initialize();

    m_system_manager.log();
}

//**************************************************************
//~JadeFrame
//**************************************************************

//**************************************************************
// Application
//**************************************************************

Application::Application(const Desc& desc)
    : m_gui() {
    Logger::info("Creating Window....");
    Instance*    i = Instance::get_singleton();
    Window::Desc win_desc = {
        .title = desc.title,
        .size = desc.size,
        .position = desc.position,
    };
    Window*            requested_window = i->m_system_manager.request_window(win_desc);
    const std::string& title = requested_window->get_title();
    std::string        new_title = title + " - " + to_string(desc.api);
    requested_window->set_title(new_title);

    std::vector<GRAPHICS_API> apis = RenderSystem::list_available_graphics_apis();
    for (GRAPHICS_API& api : apis) { Logger::info("API: '{}'", to_string(api)); }
    Logger::info("Creating Renderer with API '{}'", to_string(desc.api));
    m_render_system.init(desc.api, requested_window);
    // m_render_system = m_system_manager.request_render_system(api, requested_window);
    requested_window->add_event_callback([this](const WindowEvent& event) -> void {
        if (event.type != WindowEvent::TYPE::RESIZE) { return; }
        if (m_render_system.m_renderer == nullptr) { return; }

        const WindowResizeEvent resize_event = event.resize_event;
        m_render_system.m_renderer->set_viewport(
            0, 0, resize_event.width, resize_event.height
        );
    });

    m_gui.init(requested_window, desc.api);

    m_windows[0] = requested_window;
    m_current_window_p = m_windows[0];
}

auto Application::start() -> void {
    // Before `this->m_on_init_fn();` come all the default stuff
    // The client can later override those in `this->m_on_init_fn();`

    Window* curr_win = m_windows[0];
    auto    curr_size = curr_win->get_size();
    m_camera = Camera::orthographic(
        0, static_cast<f32>(curr_size.x), static_cast<f32>(curr_size.y), 0, -1, 1
    );
    SystemManager& platform = Instance::get_singleton()->m_system_manager;
    platform.set_target_FPS(60);
    this->m_on_init_fn();

    IRenderer* renderer = m_render_system.m_renderer.get();
    f64        previous_frame_time = platform.get_time();
    while (m_is_running) {
        auto      frame_time_start = platform.get_time();
        const f64 delta_time = frame_time_start - previous_frame_time;
        previous_frame_time = frame_time_start;

        this->m_on_update_fn();

        // if (m_current_window_p->get_window_state() != Window::WINDOW_STATE::MINIMIZED)
        // {
        renderer->clear_background();

        if (m_gui.m_is_initialized) { m_gui.new_frame(); }
        control_camera(m_camera, m_windows[0]->m_input_state, delta_time);

        this->m_on_draw_fn();

        renderer->render(m_camera);
        if (m_gui.m_is_initialized) { m_gui.render(); }

        renderer->present();
        m_tick += 1;
        //}
        this->poll_events();
        const f64  frame_time_end = platform.get_time();
        const auto frame_work_time = frame_time_end - frame_time_start;
        platform.frame_control(frame_work_time);
    }
}

auto Application::poll_events() -> void {
    for (auto& [id, window] : m_windows) { window->handle_events(m_is_running); }
}

//**************************************************************
//~Application
//**************************************************************

} // namespace JadeFrame
