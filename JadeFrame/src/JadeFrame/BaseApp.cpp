#include "BaseApp.h"
#include <iostream>
#include "GUI.h"
#include "math/Math.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../extern/stb/stb_image_write.h"

BaseApp* BaseApp::instance = nullptr;
BaseApp::BaseApp() {}

BaseApp::~BaseApp() {
	glfwTerminate();
}

auto BaseApp::init_app(const std::string& title, Vec2 size) -> void {
	instance = this;
	m_window.init(title, size);
	m_shader.init();
	m_renderer.init(&m_shader);

	m_input.setWindowInstance(m_window.get_handle());

	glfwSetMouseButtonCallback(m_window.get_handle(), m_input.mouse_button_callback);
	glfwSetCursorEnterCallback(m_window.get_handle(), m_input.cursor_enter_callback);
	glfwSetKeyCallback(m_window.get_handle(), m_input.key_callback);
}
static auto process_input(GLFWwindow* window) -> void {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}
auto glCheckError_(const char* file, int line) -> GLenum {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#include <stdint.h>

auto bytes_to_string(int64_t bytes) -> std::string {
	static const float gb = 1024 * 1024 * 1024;
	static const float mb = 1024 * 1024;
	static const float kb = 1024;

	std::string result;
	if (bytes > gb) {
		result = std::to_string(bytes / gb) + " GB";
	} else if (bytes > mb) {
		result = std::to_string(bytes / mb) + " MB";
	} else if (bytes > kb) {
		result = std::to_string(bytes / kb) + " KB";
	} else {
		result = std::to_string((float)bytes) + " bytes";
	}
	return result;
}
struct SystemMemoryInfo {
	int64_t availablePhysicalMemory;
	int64_t totalPhysicalMemory;

	int64_t availableVirtualMemory;
	int64_t totalVirtualMemory;

	auto Log() -> void {
		std::string apm, tpm, avm, tvm;

		uint32_t gb = 1024 * 1024 * 1024;
		uint32_t mb = 1024 * 1024;
		uint32_t kb = 1024;

		apm = bytes_to_string(availablePhysicalMemory);
		tpm = bytes_to_string(totalPhysicalMemory);
		avm = bytes_to_string(availableVirtualMemory);
		tvm = bytes_to_string(totalVirtualMemory);

		std::cout << std::endl;
		std::cout << "Memory Info:" << std::endl;
		std::cout << "\tPhysical Memory (Avail/Total): " << apm << " / " << tpm << std::endl;
		std::cout << "\tVirtual Memory  (Avail/Total): " << avm << " / " << tvm << std::endl;
		std::cout << std::endl;
	}
};

auto get_system_info() -> SystemMemoryInfo {
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&status);

	SystemMemoryInfo result =
	{
		(int64_t)status.ullAvailPhys,
		(int64_t)status.ullTotalPhys,

		(int64_t)status.ullAvailVirtual,
		(int64_t)status.ullTotalVirtual
	};
	return result;
}

auto BaseApp::run_app() -> void {
	GUI_init(get_window().get_handle());


	{
		std::cout << "Uniforms: ";
		for (auto uniform : m_shader.m_uniforms) {
			std::cout << uniform.name << " ";
		} std::cout << std::endl;

		std::cout << "Attribs : ";
		for (auto attributes : m_shader.m_attributes) {
			std::cout << attributes.name << " ";
		} std::cout << std::endl;
	}


	auto system_info = get_system_info();
	system_info.Log();

	std::cout << "availablePhysicalMemory: " << system_info.availablePhysicalMemory << std::endl;
	std::cout << "totalPhysicalMemory: " << system_info.totalPhysicalMemory << std::endl;
	std::cout << "availableVirtualMemory: " << system_info.availableVirtualMemory << std::endl;
	std::cout << "totalVirtualMemory: " << system_info.totalVirtualMemory << std::endl;

	static Vec3 camPos = { 0, 10, 10 };

	Camera cam;
	cam.perspective(
		camPos,
		to_radians(45.0f),
		m_window.get_width() / m_window.get_height(),
		0.1f,
		100.0f
	);
	m_renderer.matrix_stack.projection_matrix = cam.get_projection_matrix();

	glEnable(GL_DEPTH_TEST);
	m_renderer.gl_cache.set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
	while (!glfwWindowShouldClose(m_window.get_handle())) {
		glfwSwapBuffers(m_window.get_handle());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		this->poll_events();


		GUI_new_frame();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


		{


			m_renderer.start(PRIMITIVE_TYPE::LINES);
			cam.move();
			cam.update();
			m_renderer.matrix_stack.view_matrix = cam.get_view_matrix();

			for (float i = -10; i <= 10; i += 1.0f) {
				m_renderer.set_color({ 0.5f, 0.8f, 0.5f, 1.0f }); MeshManager::make_line({ i, -10, 0 }, { i, 10, 0 }).send_to_buffer(); // RED X
			}
			for (float i = -10; i <= 10; i += 1.0f) {
				m_renderer.set_color({ 0.8f, 0.5f, 0.5f, 1.0f }); MeshManager::make_line({ -10, i, 0 }, { 10, i, 0 }).send_to_buffer(); // RED X
			}

			m_renderer.set_color({ 1.0f, 0.0f, 0.0f, 1.0f }); MeshManager::make_line({ 0, 0, 0 }, { 10, 0, 0 }).send_to_buffer(); // RED X
			m_renderer.set_color({ 0.0f, 1.0f, 0.0f, 1.0f }); MeshManager::make_line({ 0, 0, 0 }, { 0, 10, 0 }).send_to_buffer(); // GREEN Y
			m_renderer.set_color({ 0.0f, 0.0f, 1.0f, 1.0f }); MeshManager::make_line({ 0, 0, 0 }, { 0, 0, 10 }).send_to_buffer(); // BLUE Z

			m_renderer.set_color({ 0.5f, 0.0f, 0.0f, 1.0f }); MeshManager::make_line({ 0, 0, 0 }, { -10, 0, 0 }).send_to_buffer(); // RED X
			m_renderer.set_color({ 0.0f, 0.5f, 0.0f, 1.0f }); MeshManager::make_line({ 0, 0, 0 }, { 0, -10, 0 }).send_to_buffer(); // GREEN Y
			m_renderer.set_color({ 0.0f, 0.0f, 0.5f, 1.0f }); MeshManager::make_line({ 0, 0, 0 }, { 0, 0, -10 }).send_to_buffer(); // BLUE Z

			m_renderer.end();
		}


		{
			PRIMITIVE_TYPE pt;
			if (m_input.is_key_down(KEY::T)) {
				pt = PRIMITIVE_TYPE::LINES;

			}
			else {
				pt = PRIMITIVE_TYPE::TRIANGLES;
			}
			m_renderer.start(pt);
			cam.move();
			cam.update();
			m_renderer.matrix_stack.view_matrix = cam.get_view_matrix();

			static Vec3 cubePos;
			ImGui::SliderFloat("cubePosX", &cubePos.x, -30, 30);
			ImGui::SliderFloat("cubePosY", &cubePos.y, -30, 30);
			ImGui::SliderFloat("cubePosZ", &cubePos.z, -30, 30);
			{
				m_renderer.set_color({ 0.0f, 0.0, 0.0f, 1.0f }); MeshManager::make_cube(cubePos, { 1.0f, 1.0f, 1.0f }).send_to_buffer(); // RED X


				m_renderer.set_color({ 1.0f, 0.0, 0.0f, 1.0f }); MeshManager::make_cube({ 5, 5, 5 }, { 1.0f, 1.0f, 100.0f }).send_to_buffer(); // RED X
				m_renderer.set_color({ 1.0f, 1.0, 0.0f, 1.0f }); MeshManager::make_cube({ 5, -5, 5 }, { 1.0f, 1.0f, 100.0f }).send_to_buffer(); // RED X
				m_renderer.set_color({ 1.0f, 0.0, 0.0f, 1.0f }); MeshManager::make_cube({ -5, 30, 5 }, { 1.0f, 1.0f, 100.0f }).send_to_buffer(); // RED X
				m_renderer.set_color({ 1.0f, 1.0, 0.0f, 1.0f }); MeshManager::make_cube({ -5, -5, 5 }, { 1.0f, 1.0f, 100.0f }).send_to_buffer(); // RED X

			
			}
			m_renderer.end();
		}


		GUI_render();

	}
}


auto BaseApp::poll_events() -> void {
	process_input(m_window.get_handle());
	m_time_manager.handle_time();
	m_input.handle_input();
	glfwPollEvents();
}

auto TimeManager::handle_time() -> void {
	// Frame time control system
	current_time = glfwGetTime();
	draw_time = current_time - previous_time;
	previous_time = current_time;

	frame_time = update_time + draw_time;

	// Wait for some milliseconds...
	if (frame_time < target_time) {
		Sleep((unsigned int)((float)(target_time - frame_time) * 1000.0f));
		current_time = glfwGetTime();
		double extra_time = current_time - previous_time;
		previous_time = current_time;
		frame_time += extra_time;
	}
}


enum class PLATTFORM {
	WINDOWS,
	MAC,
	LINUX
};


//enum class CONTEXT {
//	WGL,
//	GLX,
//	OSMESA,
//	EGL,
//	NSGL
//};
enum class GRAPHICS {
	NONE,
	OPENGL,
	DIRECTX,
};