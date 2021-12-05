#include "pch.h"
#include "base_app.h"
#include "graphics/graphics_shared.h"


#include "gui.h"

#include <utility>
//#include <format>
#include <JadeFrame/utils/utils.h>
#include "JadeFrame/math/vec.h"

namespace JadeFrame {

template<class T>
requires std::same_as<T, bool>
auto init_memory(T& data) -> void {
	static_assert(!std::is_pointer<T>::value, "'init_memory' does not allow pointer types");
	static_assert(std::is_pod<T>::value, "'init_memory' does only allow plain-old-data (POD)");
	::memset(&data, 0, sizeof(T));
}


class RenderCommandQueue {
public:
	typedef void(*RenderCommandFn)(void*);
	RenderCommandQueue() {
		const auto buffer_size = 10 * 1024 * 1024;
		m_command_buffer = new u8[buffer_size];
		m_command_buffer_ptr = m_command_buffer;
		std::memset(m_command_buffer, 0, buffer_size);
	}
	~RenderCommandQueue() {
		delete[] m_command_buffer;
	}
	auto allocate(RenderCommandFn func, u32 size) {
		// TODO: alignment
		*(RenderCommandFn*)m_command_buffer_ptr = func;
		m_command_buffer_ptr += sizeof(RenderCommandFn);

		*(u32*)m_command_buffer_ptr = size;
		m_command_buffer_ptr += sizeof(u32);

		void* memory = m_command_buffer_ptr;
		m_command_buffer_ptr += size;

		m_command_count++;
		return memory;
	}
	auto execute() -> void {

		u8* buffer = m_command_buffer;

		for (uint32_t i = 0; i < m_command_count; i++) {
			RenderCommandFn function = *(RenderCommandFn*)buffer;
			buffer += sizeof(RenderCommandFn);

			u32 size = *(u32*)buffer;
			buffer += sizeof(u32);
			function(buffer);
			buffer += size;
		}

		m_command_buffer_ptr = m_command_buffer;
		m_command_count = 0;
	}
private:
	u8* m_command_buffer;
	u8* m_command_buffer_ptr;
	u32 m_command_count = 0;
};



//**************************************************************
//JadeFrame
//**************************************************************
JadeFrameInstance* JadeFrameInstance::m_singleton = nullptr;
auto JadeFrameInstance::get_singleton() -> JadeFrameInstance* {
	return m_singleton;
}

template<typename FuncT>
static auto submit(FuncT&& func) -> void {
	RenderCommandQueue rcq;
	auto render_cmd = [](void* ptr) {
		FuncT* p_func = (FuncT*)ptr;
		(*p_func)();


		p_func->~FuncT();
	};
	auto storage_buffer = rcq.allocate(render_cmd, sizeof(func));
	new(storage_buffer) FuncT(std::forward<FuncT>(func));




	//__debugbreak();
	rcq.execute();
	//__debugbreak();
}

JadeFrameInstance::JadeFrameInstance() {
	Logger::init();
	Logger::info("JadeFrame is starting...");

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

	Windows_Window::Desc win_desc;
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

	this->on_init();
	//m_renderer->main_loop();
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

struct Error {
	std::error_code type;
	//VkResult vk_result = VK_SUCCESS; // optional error value if a vulkan call failed
};
template<typename T>
class Result {
public:
	Result(const T& value)
		: m_value{ value }
		, m_init{ true } {}

	Result(T&& value)
		: m_value{ std::move(value) }
		, m_init{ true } {}

	Result(Error error)
		: m_error{ error }
		, m_init{ false } {}

	Result(std::error_code error_code, VkResult result = VK_SUCCESS)
		: m_error{ error_code, result }
		, m_init{ false } {}

	~Result() {
		destroy();
	}
	Result(Result const& expected)
		: m_init(expected.m_init) {
		if (m_init)
			new (&m_value) T{ expected.m_value };
		else
			m_error = expected.m_error;
	}
	Result(Result&& expected) : m_init(expected.m_init) {
		if (m_init)
			new (&m_value) T{ std::move(expected.m_value) };
		else
			m_error = std::move(expected.m_error);
		expected.destroy();
	}

	Result& operator= (const T& expect) {
		destroy();
		m_init = true;
		new (&m_value) T{ expect };
		return *this;
	}
	Result& operator= (T&& expect) {
		destroy();
		m_init = true;
		new (&m_value) T{ std::move(expect) };
		return *this;
	}
	Result& operator= (const Error& error) {
		destroy();
		m_init = false;
		m_error = error;
		return *this;
	}
	Result& operator= (Error&& error) {
		destroy();
		m_init = false;
		m_error = error;
		return *this;
	}
	// clang-format off
	const T* operator-> () const {
		assert(m_init);
		return &m_value;
	}
	T* operator-> () {
		assert(m_init);
		return &m_value;
	}
	const T& operator* () const& {
		assert(m_init);
		return m_value;
	}
	T& operator* ()& {
		assert(m_init);
		return m_value;
	}
	T&& operator* ()&& {
		assert(m_init);
		return std::move(m_value);
	}
	const T& value() const& {
		assert(m_init);
		return m_value;
	}
	T& value()& {
		assert(m_init);
		return m_value;
	}
	const T&& value() const&& {
		assert(m_init);
		return std::move(m_value);
	}
	T&& value()&& {
		assert(m_init);
		return std::move(m_value);
	}

	std::error_code error() const {
		assert(!m_init);
		return m_error.type;
	}
	// clang-format on


	bool has_value() const {
		return m_init;
	}
	explicit operator bool() const {
		return m_init;
	}


private:
	void destroy() {
		if (m_init) m_value.~T();
	}
	union {
		T m_value;
		Error m_error;
	};
	bool m_init;
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