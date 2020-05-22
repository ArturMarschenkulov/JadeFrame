#include "BaseApp.h"
#include <iostream>
#include "GUI.h"
#include "math/Math.h"
#include <stdint.h>
#include <Windows.h>
#include "FontManager.h"




BaseApp* BaseApp::instance = nullptr;
BaseApp::BaseApp() {}

BaseApp::~BaseApp() {
}

auto BaseApp::init_app(const std::string& title, Vec2 size) -> void {
	instance = this;
	m_window.init(title, size);
	m_shader = std::make_unique<GLShader>();
	m_shader->init();
	m_renderer = std::make_unique<GLBatchRenderer>();
	m_renderer->init(m_shader.get());

	//m_input.setWindowInstance(m_window.get_handle());
}



auto print_GPU_variable_info() -> void {

	auto& m_shader = BaseApp::get_app_instance()->m_shader;

	std::cout << "Uniforms: ";
	for (auto uniform : m_shader->m_uniforms) {
		std::cout << uniform.name << " ";
	} std::cout << std::endl;

	std::cout << "Attribs : ";
	for (auto attributes : m_shader->m_attributes) {
		std::cout << attributes.name << " ";
	} std::cout << std::endl;
}

struct CPUInfo {


	auto get_info() -> void {
		SYSTEM_INFO siSysInfo;
		GetSystemInfo(&siSysInfo);
		OEM_ID = siSysInfo.dwOemId;
		number_of_processors = siSysInfo.dwNumberOfProcessors;
		page_size = siSysInfo.dwPageSize;
		processor_type = siSysInfo.dwProcessorType;
		minimum_application_address = siSysInfo.lpMinimumApplicationAddress;
		maximum_application_address = siSysInfo.lpMaximumApplicationAddress;
		active_processor_mask = siSysInfo.dwActiveProcessorMask;
	}
	auto print() -> void {
		//printf("Hardware information: \n");
		//printf("  OEM ID: %u\n", OEM_ID);
		//printf("  Number of processors: %u\n", number_of_processors);
		//printf("  Page size: %u\n", page_size);
		//printf("  Processor type: %u\n", processor_type);
		//printf("  Minimum application address: %lx\n", minimum_application_address);
		//printf("  Maximum application address: %lx\n", maximum_application_address);
		//printf("  Active processor mask: %u\n", active_processor_mask);
	}
	DWORD OEM_ID;
	DWORD page_size;
	DWORD processor_type;
	DWORD number_of_processors;
	LPVOID minimum_application_address;
	LPVOID maximum_application_address;
	DWORD_PTR active_processor_mask;
};
struct GPUInfo {
	auto get_info() -> void {
		vendor = reinterpret_cast<char const*>(glGetString(GL_VENDOR));
		renderer = reinterpret_cast<char const*>(glGetString(GL_RENDERER));
		version = reinterpret_cast<char const*>(glGetString(GL_VERSION));
		shading_language_version = reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
		glGetIntegerv(GL_MAJOR_VERSION, &major_version);
		glGetIntegerv(GL_MINOR_VERSION, &minor_version);
		glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
		glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
		glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
		glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
		for (int i = 0; i < num_extensions; i++) {
			extentenions.push_back(reinterpret_cast<char const*>(glGetStringi(GL_EXTENSIONS, i)));
		}
	}
	auto print() -> void {
		std::cout << "GL_VENDOR: " << vendor << std::endl;
		std::cout << "GL_RENDERER: " << renderer << std::endl;
		std::cout << "GL_VERSION: " << version << std::endl;
		std::cout << "GL_SHADING_LANGUAGE_VERSION: " << shading_language_version << std::endl;

		std::cout << "GL_MAJOR_VERSION: " << major_version << std::endl;
		std::cout << "GL_MINOR_VERSION: " << minor_version << std::endl;
		std::cout << "GL_MAX_DRAW_BUFFERS: " << max_draw_buffers << std::endl;
		std::cout << "GL_MAX_CLIP_DISTANCES: " << max_clip_distances << std::endl;
		std::cout << "GL_NUM_EXTENSIONS: " << num_extensions << std::endl;
		for (int i = 0; i < num_extensions; i++) {
			std::cout << "GL_EXTENSIONS " << i << ": " << extentenions[i] << std::endl;
		}
	}
	std::string vendor;
	std::string renderer;
	std::string version;
	std::string shading_language_version;
	std::vector<std::string> extentenions;
	int major_version;
	int minor_version;
	int max_clip_distances;
	int max_draw_buffers;
	int num_extensions;
};

auto BaseApp::run_app() -> void {
	FontManager fm;
	fm.init();
	GUI_init(get_window().m_window_handle);
	print_GPU_variable_info();
	//CPUInfo cpu_info;
	//cpu_info.get_info();
	//cpu_info.print();
	//GPUInfo gpu_info;
	//gpu_info.get_info();
	//gpu_info.print();




	//const int size = 1000;
	static Vec3 camPos = { 10, 10, 10 };
	m_renderer->cam.perspective(
		camPos,
		to_radians(45.0f),
		m_window.get_width() / m_window.get_height(),
		0.1f,
		100.0f
	);
	m_renderer->matrix_stack.projection_matrix = m_renderer->cam.get_projection_matrix();


	m_renderer->gl_cache.set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
	HDC device_context = GetDC(m_window.m_window_handle);


	while (get_window().m_is_running) {
		SwapBuffers(device_context);


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		this->poll_events();


		GUI_new_frame();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		{

			{
				m_renderer->start(PRIMITIVE_TYPE::LINES);
				//m_renderer->cam.move();
				//m_renderer->cam.update();


				for (float i = -10; i <= 10; i += 1.0f) {
					m_renderer->set_color({ 0.5f, 0.8f, 0.5f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_line({ i, -10, 0 }, { i, 10, 0 }));//.send_to_buffer(); // RED X
				}
				for (float i = -10; i <= 10; i += 1.0f) {
					m_renderer->set_color({ 0.8f, 0.5f, 0.5f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_line({ -10, i, 0 }, { 10, i, 0 }));//.send_to_buffer(); // RED X
				}

				m_renderer->set_color({ 1.0f, 0.0f, 0.0f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 10, 0, 0 }));//.send_to_buffer(); // RED X
				m_renderer->set_color({ 0.0f, 1.0f, 0.0f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, 10, 0 }));//.send_to_buffer(); // GREEN Y
				m_renderer->set_color({ 0.0f, 0.0f, 1.0f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, 0, 10 }));//.send_to_buffer(); // BLUE Z

				m_renderer->set_color({ 0.5f, 0.0f, 0.0f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { -10, 0, 0 }));//.send_to_buffer(); // RED X
				m_renderer->set_color({ 0.0f, 0.5f, 0.0f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, -10, 0 }));//.send_to_buffer(); // GREEN Y
				m_renderer->set_color({ 0.0f, 0.0f, 0.5f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, 0, -10 }));//.send_to_buffer(); // BLUE Z


				m_renderer->end();
			}


			{
				PRIMITIVE_TYPE pt = PRIMITIVE_TYPE::TRIANGLES;

				m_renderer->start(pt);
				//m_renderer->cam.move();
				//m_renderer->cam.update();


				static Vec3 cubePos;
				ImGui::SliderFloat("cubePosX", &cubePos.x, -30, 30);
				ImGui::SliderFloat("cubePosY", &cubePos.y, -30, 30);
				ImGui::SliderFloat("cubePosZ", &cubePos.z, -30, 30);
				//if (m_input_manager.is_key_press(EKey::B))
				{
					m_renderer->set_color({ 0.0f, 0.0, 0.0f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_cube(cubePos, { 1.0f, 1.0f, 1.0f }));//.send_to_buffer(); // RED X


					m_renderer->set_color({ 1.0f, 0.0, 0.0f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_cube({ 5, 5, 5 }, { 1.0f, 1.0f, 100.0f }));//.send_to_buffer(); // RED X
					m_renderer->set_color({ 1.0f, 1.0, 0.0f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_cube({ 5, -5, 5 }, { 1.0f, 1.0f, 100.0f }));//.send_to_buffer(); // YELLOW X
					m_renderer->set_color({ 1.0f, 0.0, 1.0f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_cube({ -5, 30, 5 }, { 1.0f, 1.0f, 100.0f }));//.send_to_buffer(); // VIOLET X
					m_renderer->set_color({ 1.0f, 1.0, 1.0f, 1.0f }); m_renderer->add_to_buffer(MeshManager::make_cube({ -5, -5, 5 }, { 1.0f, 1.0f, 100.0f }));//.send_to_buffer(); // WHITE X


				}

				m_renderer->end();
			}
		}
		GUI_render();
	}
}


auto BaseApp::poll_events() -> void {
	m_input_manager.handle_input();

	MSG message;
	while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
		if (message.message == WM_QUIT) {
			m_window.m_is_running = false;
			return;
		}
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}



