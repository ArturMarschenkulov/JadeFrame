#include "BaseApp.h"
#include <iostream>
#include "GUI.h"
#include "math/Math.h"
#include <stdint.h>
#include <Windows.h>
#include "FontManager.h"




static auto add_grid_to_buffer(GLBatchBufferData& buffer) -> void {
	for (int i = -10; i <= 10; i += 1) {
		buffer.set_color({ 0.5f, 0.8f, 0.5f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ static_cast<float>(i), -10, 0 }, { static_cast<float>(i), 10, 0 }));//.send_to_buffer(); // RED X
	}
	for (int i = -10; i <= 10; i += 1) {
		buffer.set_color({ 0.8f, 0.5f, 0.5f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ -10, static_cast<float>(i), 0 }, { 10, static_cast<float>(i), 0 }));//.send_to_buffer(); // RED X
	}
}
static auto add_xyz_lines_to_buffer(GLBatchBufferData& buffer) -> void {
	buffer.set_color({ 1.0f, 0.0f, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 10, 0, 0 }));// RED X
	buffer.set_color({ 0.0f, 1.0f, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, 10, 0 }));// GREEN Y
	buffer.set_color({ 0.0f, 0.0f, 1.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, 0, 10 }));// BLUE Z

	buffer.set_color({ 0.5f, 0.0f, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { -10, 0, 0 })); // RED X
	buffer.set_color({ 0.0f, 0.5f, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, -10, 0 })); // GREEN Y
	buffer.set_color({ 0.0f, 0.0f, 0.5f, 1.0f }); buffer.add_to_buffer(MeshManager::make_line({ 0, 0, 0 }, { 0, 0, -10 })); // BLUE Z
}
static auto add_cubes_to_buffer(GLBatchBufferData& buffer) -> void {
	static Vec3 cubePos;
	{
		buffer.set_color({ 0.0f, 0.0, 0.0f, 0.5f }); buffer.add_to_buffer(MeshManager::make_cube(cubePos, { 1.0f, 1.0f, 1.0f }));// BLACK X

		 
		buffer.set_color({ 1.0f, 0.0, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_cube({ 5, 5, 5 }, { 1.0f, 1.0f, 100.0f }));   // RED X
		buffer.set_color({ 1.0f, 1.0, 0.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_cube({ 5, -5, 5 }, { 1.0f, 1.0f, 100.0f }));  // YELLOW X
		buffer.set_color({ 1.0f, 0.0, 1.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_cube({ -5, 30, 5 }, { 1.0f, 1.0f, 100.0f })); // VIOLET X
		buffer.set_color({ 1.0f, 1.0, 1.0f, 1.0f }); buffer.add_to_buffer(MeshManager::make_cube({ -5, -5, 5 }, { 1.0f, 1.0f, 100.0f })); // WHITE X
	}
}

static auto add_cube_to_buffer(GLBatchBufferData& buffer) -> void {
	buffer.set_color({ 0.0f, 0.0, 0.0f, 0.5f }); buffer.add_to_buffer(MeshManager::make_cube({0, 0, 0}, { 1.0f, 1.0f, 1.0f }));// BLACK X
}

static auto add_texture_plane(GLBatchBufferData& buffer) -> void {
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

static auto do_render_stuff(GLRenderer& renderer) -> void {
	renderer.start(PRIMITIVE_TYPE::TRIANGLES);
	//add_cube_to_buffer(renderer.buffer_data);
	add_texture_plane(renderer.buffer_data);
	renderer.end();
}

BaseApp* BaseApp::instance = nullptr;
BaseApp::BaseApp() {
}

BaseApp::~BaseApp() {
}

auto BaseApp::start(const std::string& title, Vec2 size) -> void {
	instance = this;
	m_window.init(title, size);

	//Renderer
	GLRenderer renderer;
	renderer.init();
	renderer.gl_cache.set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
	renderer.gl_cache.set_depth_test(true);
	renderer.gl_cache.set_clear_bitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	renderer.gl_cache.set_blending(true);
	//~Renderer

	//Shader
	GLShader shader;
	shader.init();
	renderer.current_shader = &shader;
	renderer.current_shader->use();
	//~Shader

	//Camera
	Camera camera;
	camera.perspective(
		{ -20, 10, -5 },
		to_radians(45.0f),
		m_window.m_size.x / m_window.m_size.y,
		0.1f,
		100.0f
	);
	renderer.matrix_stack.projection_matrix = camera.get_projection_matrix();
	renderer.cam = camera;
	//~Camera


	//BufferData
	GLBatchBufferData buffer_data;
	buffer_data.init();
	renderer.buffer_data = buffer_data;
	//~BufferData

	//Mesh cube_mesh = MeshManager::make_cube({ 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });
	//GLBatchBufferData cube_mesh_buffer;
	//cube_mesh_buffer.init();
	//cube_mesh_buffer.set_color({ 0.0f, 1.0, 0.0f, 1.0f }); cube_mesh_buffer.add_to_buffer(cube_mesh);
	//cube_mesh_buffer.update();

	Mesh cube_mesh = MeshManager::make_cube({ 0, 0, 0 }, { 1.0f, 1.0f, 1.0f });
	GLBufferData cube_mesh_buffer;
	cube_mesh_buffer.set_color({ 0.0f, 1.0, 0.0f, 1.0f });
	cube_mesh_buffer.init(cube_mesh);



	std::deque<GLBufferData> buffers;
	//buffers.reserve(4);
	for (int i = 0; i < 10; i++) {
		Mesh mesh = MeshManager::make_cube({ 1.0f * i, 1.0f * i, 1.0f * i }, { 1.0f, 1.0f, 1.0f });
		buffers.emplace_back();
		buffers[i].set_color({ 0.0f, 1.0, 0.0f, 1.0f });
		buffers[i].init(mesh);
	}	

	//GUI
 	GUI_init(m_window.m_window_handle);
	//~GUI

	HDC device_context = GetDC(m_window.m_window_handle);
	while (m_window.m_is_running) {
		SwapBuffers(device_context);
		this->clear(renderer.gl_cache.clear_bitfield);
		this->poll_events();
		GUI_new_frame();

		cube_mesh_buffer.m_primitive_type = PRIMITIVE_TYPE::TRIANGLES;
		shader.use();
		renderer.cam.move();

		{
			

			renderer.matrix_stack.view_matrix = renderer.cam.get_view_matrix();
			Mat4 MVP = renderer.matrix_stack.view_matrix * renderer.matrix_stack.projection_matrix;
			shader.set_uniform_matrix("MVP", MVP);

			for(int i = 0; i < buffers.size(); i++) {
				buffers[i].draw();
			}

			//cube_mesh_buffer.draw();
		}

		renderer.start(PRIMITIVE_TYPE::TRIANGLES);
		//add_cube_to_buffer(renderer.buffer_data);
		//add_texture_plane(renderer.buffer_data);
		renderer.end();


		draw_GUI(&renderer);
		GUI_render();
	}

}
auto BaseApp::poll_events() -> void {
	m_input_manager.handle_input();

	MSG message;
	while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
		if (message.message == WM_QUIT) {
			//m_window.m_is_running = false;
			return;
		}
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}
auto BaseApp::clear(GLbitfield bitfield) -> void {
	glClear(bitfield);
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
		printf("Hardware information: \n");
		printf("  OEM ID: %u\n", OEM_ID);
		printf("  Number of processors: %u\n", number_of_processors);
		printf("  Page size: %u\n", page_size);
		printf("  Processor type: %u\n", processor_type);
		printf("  Minimum application address: %lx\n", minimum_application_address);
		printf("  Maximum application address: %lx\n", maximum_application_address);
		printf("  Active processor mask: %u\n", active_processor_mask);
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


struct Actor {
};