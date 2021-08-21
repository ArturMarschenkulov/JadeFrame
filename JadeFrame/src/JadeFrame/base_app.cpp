#include "pch.h"
#include "base_app.h"
#include "graphics/shared.h"


#include "gui.h"

#include <utility>
#include <format>

namespace JadeFrame {
//**************************************************************
//JadeFrame
//**************************************************************
JadeFrameInstance* JadeFrameInstance::m_singleton = nullptr;
auto JadeFrameInstance::get_singleton() -> JadeFrameInstance* {
	return m_singleton;
}
JadeFrameInstance::JadeFrameInstance() {
	Logger::log("JadeFrame is starting...");

	if (m_singleton == nullptr) {
		m_singleton = this;
		m_system_manager.initialize();
		//m_time_manager.initialize();
		//m_input_manager.initialize();

		m_system_manager.log();
	} else {
		__debugbreak();
	}
}
auto JadeFrameInstance::run() -> void {
	m_current_app_p = m_apps[0];
	m_apps.back()->start();
}
auto JadeFrameInstance::add(BaseApp* app) -> void {
	m_apps.push_back(app);
}
//**************************************************************
//~JadeFrame
//**************************************************************

//**************************************************************
//BaseApp
//**************************************************************

BaseApp::BaseApp(DESC desc) {
	m_time_manager.initialize();

	Windows_Window::DESC win_desc;
	win_desc.title = desc.title;
	win_desc.size = desc.size;
	win_desc.position = desc.position;
	m_windows.try_emplace(0, win_desc);

	m_current_window_p = &m_windows[0];

	enum class API {
		OPENGL,
		VULKAN,
	};
	API api;
	api = API::VULKAN;

	switch (api) {
		case API::OPENGL:
		{
			m_renderer = new OpenGL_Renderer(m_windows[0]);
		} break;
		case API::VULKAN:
		{
			m_renderer = new Vulkan_Renderer(m_windows[0]);
		} break;
	}

}
auto BaseApp::start() -> void {
	m_renderer->main_loop();

	this->on_init();

	//GUI_init(m_current_window_p->m_window_handle);

	m_time_manager.set_FPS(60);
	while (m_is_running) {

		this->on_update();
		if (m_current_window_p->get_window_state() != Windows_Window::WINDOW_STATE::MINIMIZED) {
			const f64 time_since_last_frame = m_time_manager.calc_elapsed();
			m_renderer->clear_background();
			//GUI_new_frame();



			this->on_draw();
			const Matrix4x4& view_projection = m_camera.get_view_projection_matrix();
			m_renderer->render(view_projection);


			//GUI_render();


			m_renderer->present();
			m_time_manager.frame_control(time_since_last_frame);
		}
		this->poll_events();
	}
}
auto BaseApp::poll_events() -> void {
	JadeFrameInstance::get_singleton()->m_input_manager.handle_input();

	//TODO: Abstract the Windows code away
	MSG message;
	while (::PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
		if (message.message == WM_QUIT) {
			this->m_is_running = false;
			return;
		}
		::TranslateMessage(&message);
		::DispatchMessageW(&message);
	}
}
//**************************************************************
//~BaseApp
//**************************************************************

}