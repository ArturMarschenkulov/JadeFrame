#include "pch.h"
#include "base_app.h"
#include "graphics/shared.h"


#include "gui.h"

#include <utility>
#include <format>
#include <JadeFrame/utils/utils.h>

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

BaseApp::BaseApp(const DESC& desc) {
	m_time_manager.initialize();

	Windows_Window::DESC win_desc;
	win_desc.title = desc.title;
	win_desc.size = desc.size;
	win_desc.position = desc.position;
	m_windows.try_emplace(0, win_desc);

	m_current_window_p = &m_windows[0];

	GRAPHICS_API api = GRAPHICS_API::UNDEFINED;
	api = GRAPHICS_API::VULKAN;
	//api = GRAPHICS_API::OPENGL;

	const std::string& title = m_current_window_p->get_title();
	switch (api) {
		case GRAPHICS_API::OPENGL:
		{
			m_renderer = new OpenGL_Renderer(m_windows[0]);
			m_current_window_p->set_title(title + " OpenGL");

		} break;
		case GRAPHICS_API::VULKAN:
		{
			m_renderer = new Vulkan_Renderer(m_windows[0]);
			m_current_window_p->set_title(title + " Vulkan");
		} break;
	}


}
auto BaseApp::start() -> void {
	m_renderer->main_loop();

	this->on_init();

	//GUI_init(m_current_window_p->m_window_handle);
	m_time_manager.set_FPS(60);
	while (m_is_running) {
		const f64 time_since_last_frame = m_time_manager.calc_elapsed();
		this->on_update();
		if (m_current_window_p->get_window_state() != Windows_Window::WINDOW_STATE::MINIMIZED) {
			m_renderer->clear_background();
			//GUI_new_frame();



			this->on_draw();
			const Matrix4x4& view_projection = m_camera.get_view_projection_matrix();
			m_renderer->render(view_projection);


			//GUI_render();


			m_renderer->present();
		}
		this->poll_events();
		m_time_manager.frame_control(time_since_last_frame);
	}
}
auto BaseApp::poll_events() -> void {
	JadeFrameInstance::get_singleton()->m_input_manager.handle_input();

	//TODO: Abstract the Windows code away
	::MSG message;
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

namespace T1 {
template <typename BaseType, typename SubType>
static auto take_ownership(std::set<std::unique_ptr<BaseType>>& object_set, std::unique_ptr<SubType>&& object) -> SubType* {
	SubType* ref = object.get();
	object_set.emplace(std::forward<std::unique_ptr<SubType>>(object));
	return ref;
}
template <typename BaseType, typename SubType>
static auto take_ownership(std::vector<std::unique_ptr<BaseType>>& object_set, std::unique_ptr<SubType>&& object) -> SubType* {
	SubType* ref = object.get();
	object_set.emplace_back(std::forward<std::unique_ptr<SubType>>(object));
	return ref;
}
template <typename BaseType, typename SubType>
static auto take_ownership(std::list<std::unique_ptr<BaseType>>& object_set, std::unique_ptr<SubType>&& object) -> SubType* {
	SubType* ref = object.get();
	object_set.emplace_back(std::forward<std::unique_ptr<SubType>>(object));
	return ref;
}

template<typename Left, typename Right>
class Either {
public:
	Either(const Left& left) {
		m_left = left;
		m_is_left = true;
	}
	Either(const Right& right) {
		m_right = right;
		m_is_left = false;
	}

	//auto match() -> T {
	//
	//}
private:
	//Left m_left;
	//Right m_right;
	union {
		Left m_left;
		Right m_right;
	};
	bool m_is_left;
};


constexpr static auto hash(const char* str) -> size_t {
	const i64 p = 131;
	const i64 m = 4294967291; // 2^32 - 5, largest 32 bit prime
	i64 total = 0;
	i64 current_multiplier = 1;
	for (i64 i = 0; str[i] != '\0'; ++i) {
		total = (total + current_multiplier * str[i]) % m;
		current_multiplier = (current_multiplier * p) % m;
	}
	return total;
}

auto foo() -> void {
	Either<i32, f32> e = 32.0f;

	auto ee = Either<i32, f32>(29);

}

template<typename T>
class UniquePointer {

};
}


}