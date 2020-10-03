#include "BaseApp.h"
#include <iostream>
#include "GUI.h"
#include "math/Math.h"
#include <stdint.h>
#include <Windows.h>
#include "FontManager.h"


auto print_GPU_variable_info() -> void;




static auto add_grid_to_buffer(GLBufferData& buffer) -> void {
	for (int i = -10; i <= 10; i += 1) {
		buffer.set_color({ 0.5f, 0.8f, 0.5f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ static_cast<float>(i), -10, 0 }, { static_cast<float>(i), 10, 0 }));//.send_to_buffer(); // RED X
	}
	for (int i = -10; i <= 10; i += 1) {
		buffer.set_color({ 0.8f, 0.5f, 0.5f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ -10, static_cast<float>(i), 0 }, { 10, static_cast<float>(i), 0 }));//.send_to_buffer(); // RED X
	}
}
static auto add_xyz_lines_to_buffer(GLBufferData& buffer) -> void {
	buffer.set_color({ 1.0f, 0.0f, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 10, 0, 0 }));// RED X
	buffer.set_color({ 0.0f, 1.0f, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, 10, 0 }));// GREEN Y
	buffer.set_color({ 0.0f, 0.0f, 1.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, 0, 10 }));// BLUE Z

	buffer.set_color({ 0.5f, 0.0f, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { -10, 0, 0 })); // RED X
	buffer.set_color({ 0.0f, 0.5f, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, -10, 0 })); // GREEN Y
	buffer.set_color({ 0.0f, 0.0f, 0.5f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, 0, -10 })); // BLUE Z
}
static auto add_cubes_to_buffer(GLBufferData& buffer) -> void {
	static Vec3 cubePos;
	//ImGui::SliderFloat("cubePosX", &cubePos.x, -30, 30);
	//ImGui::SliderFloat("cubePosY", &cubePos.y, -30, 30);
	//ImGui::SliderFloat("cubePosZ", &cubePos.z, -30, 30);

	//if (m_input_manager.is_key_press(EKey::B))
	{
		buffer.set_color({ 0.0f, 0.0, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_cube(cubePos, { 1.0f, 1.0f, 1.0f }));//.send_to_buffer(); // RED X

		 
		buffer.set_color({ 1.0f, 0.0, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_cube({ 5, 5, 5 }, { 1.0f, 1.0f, 100.0f }));   // RED X
		buffer.set_color({ 1.0f, 1.0, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_cube({ 5, -5, 5 }, { 1.0f, 1.0f, 100.0f }));  // YELLOW X
		buffer.set_color({ 1.0f, 0.0, 1.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_cube({ -5, 30, 5 }, { 1.0f, 1.0f, 100.0f })); // VIOLET X
		buffer.set_color({ 1.0f, 1.0, 1.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_cube({ -5, -5, 5 }, { 1.0f, 1.0f, 100.0f })); // WHITE X
	}
}

static auto add_texture_plane(GLBufferData& buffer) -> void {
	buffer.set_color({ 1.0f, 1.0, 1.0f, 0.1f }); buffer.add_to_buffer(MeshManager::make_rectangle({ 5, -5, -5 }, { 5, 5, 5 }));
}

auto draw_GUI(GLRenderer* m_renderer) -> void {
	ImGui::BeginMainMenuBar();

	ImGui::EndMainMenuBar();


	static bool show = true;
	ImGui::ShowDemoWindow(&show);


	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//ImGui::Checkbox("Depth Test", &b_depth_test);

	//auto& m_renderer = BaseApp::get_app_instance()->m_renderer;

	ImGui::SliderFloat("cam.m_position.x", &m_renderer->cam.m_position.x, -30, 30);
	ImGui::SliderFloat("cam.m_position.y", &m_renderer->cam.m_position.y, -30, 30);
	ImGui::SliderFloat("cam.m_position.z", &m_renderer->cam.m_position.z, -30, 30);

	ImGui::SliderFloat("cam.m_right.x", &m_renderer->cam.m_right.x, -30, 30);
	ImGui::SliderFloat("cam.m_right.y", &m_renderer->cam.m_right.y, -30, 30);
	ImGui::SliderFloat("cam.m_right.z", &m_renderer->cam.m_right.z, -30, 30);

	ImGui::SliderFloat("cam.m_fovy", &m_renderer->cam.m_fovy, to_radians(-60), to_radians(60));
}

BaseApp* BaseApp::instance = nullptr;
BaseApp::BaseApp() {
}

BaseApp::~BaseApp() {
}

auto BaseApp::start(const std::string& title, Vec2 size) -> void {
	instance = this;
	m_window.init(title, size);

	//Shader
	GLShader shader;
	shader.init();
	//~Shader

	//Renderer
	GLRenderer renderer;
	renderer.init();
	renderer.gl_cache.set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
	renderer.gl_cache.set_depth_test(true);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	renderer.current_shader = &shader;
	renderer.current_shader->use();
	//~Renderer

	//Camera
	Camera camera;
	camera.perspective(
		{ 10, 10, 10 },
		to_radians(45.0f),
		m_window.m_size.x / m_window.m_size.y,
		0.1f,
		100.0f
	);
	renderer.matrix_stack.projection_matrix = camera.get_projection_matrix();
	renderer.cam = camera;
	//~Camera

	//GUI
	GUI_init(m_window.m_window_handle);
	//~GUI
	HDC device_context = GetDC(m_window.m_window_handle);
	while (m_window.m_is_running) {
		SwapBuffers(device_context);
		this->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


		this->poll_events();
		GUI_new_frame();

		draw_GUI(&renderer);

		renderer.start(PRIMITIVE_TYPE::LINES);
		add_grid_to_buffer(renderer.buffer_data);
		add_xyz_lines_to_buffer(renderer.buffer_data);
		renderer.end();

		renderer.start(PRIMITIVE_TYPE::TRIANGLES);
		add_cubes_to_buffer(renderer.buffer_data);
		add_texture_plane(renderer.buffer_data);
		renderer.end();

		GUI_render();
	}

}

//auto BaseApp::init(const std::string& title, Vec2 size) -> void {
//	instance = this;
//	m_window.init(title, size);
//	m_renderer = std::make_unique<GLRenderer>();
//	m_renderer->init();
//}

//auto BaseApp::run() -> void {
//	GUI_init(get_window().m_window_handle);
//	static Vec3 camPos = { 10, 10, 10 };
//	m_renderer->cam.perspective(
//		camPos,
//		to_radians(45.0f),
//		m_window.get_width() / m_window.get_height(),
//		0.1f,
//		100.0f
//	);
//	m_renderer->matrix_stack.projection_matrix = m_renderer->cam.get_projection_matrix();
//
//	m_renderer->gl_cache.set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
//
//	HDC device_context = GetDC(m_window.m_window_handle);
//
//	m_renderer->gl_cache.set_depth_test(true);
//	//m_renderer->gl_cache.set_blending(true);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	
//	while (get_window().m_is_running) {
//		SwapBuffers(device_context);
//		this->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
//		
//
//		this->poll_events();
//		GUI_new_frame();
//
//		draw_GUI();
//		
//		m_renderer->start(PRIMITIVE_TYPE::LINES);
//			add_grid_to_buffer(m_renderer->buffer_data);
//			add_xyz_lines_to_buffer(m_renderer->buffer_data);
//		m_renderer->end();
//
//		m_renderer->start(PRIMITIVE_TYPE::TRIANGLES);
//			add_cubes_to_buffer(m_renderer->buffer_data);
//			add_texture_plane(m_renderer->buffer_data);
//		m_renderer->end();
//		
//		GUI_render();
//	}
//}


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

auto BaseApp::clear(GLbitfield bitfield) -> void {
	glClear(bitfield);
}


auto print_GPU_variable_info(GLShader* m_shader) -> void {

	//auto& m_shader = BaseApp::get_app_instance()->m_renderer->m_shader;

	std::cout << "Uniforms: ";
	for (auto uniform : m_shader->m_variables[0]) {
		std::cout << uniform.name << " ";
	} std::cout << std::endl;

	std::cout << "Attribs : ";
	for (auto attributes : m_shader->m_variables[1]) {
		std::cout << attributes.name << " ";
	} std::cout << std::endl;
}





struct CPUInfo {
	CPUInfo() {
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
	GPUInfo() {
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

enum class OPERATING_SYSTEM {
	WINDOWS,
	LINUX,
	MAC,
};

enum class RENDER_API {
	OPENGL,
	VULKAN,
	DIRECTX11,
	DIRECTX12,
	METAL,
	SOFTWARE,
};