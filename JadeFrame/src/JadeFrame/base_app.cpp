#include "base_app.h"

#include "gui.h"
#include "math/Math.h"

#include "graphics/opengl/opengl_texture.h"

#include <stdint.h>
#include <iostream>

static Vec4 object_color = { 0.5f, 0.5f, 0.5f, 1.0f };
static Vec3 light_color = { 1.0f, 1.0f, 1.0f };

static auto draw_GUI(TestApp& app) -> void {
	ImGui::BeginMainMenuBar();

	ImGui::EndMainMenuBar();


	static bool show = true;
	ImGui::ShowDemoWindow(&show);


	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//ImGui::Checkbox("Depth Test", &b_depth_test);

	//auto& m_renderer = BaseApp::get_instance()->m_renderer;


	//ImGui::SliderFloat("cam.m_fovy", &camera.m_fovy, to_radians(-60), to_radians(60));
	static float point_size = 1;
	ImGui::SliderFloat("point size", &point_size, 0, 100);
	glPointSize(point_size);

	static float line_width = 1;
	ImGui::SliderFloat("line width", &line_width, 0, 100);
	glLineWidth(line_width);

	static Vec3 curr_vec = Vec3(app.m_objs[1].m_transform * Vec4(app.m_objs[1].m_mesh->m_positions[0], 1.0f));
	auto prev_vec = curr_vec;

	ImGui::SliderFloat("vec_x", &curr_vec.x, -30, +30);
	ImGui::SliderFloat("vec_y", &curr_vec.y, -30, +30);
	ImGui::SliderFloat("vec_z", &curr_vec.z, -30, +30);

	if (prev_vec != curr_vec) {
		app.m_objs[1].m_transform = Mat4::translate(curr_vec);
		app.m_resources.get_shader("light_client").bind();
		const Vec3 n = Vec3(app.m_objs[1].m_transform * Vec4(app.m_objs[1].m_mesh->m_positions[0], 1.0f));
		app.m_resources.get_shader("light_client").set_uniform("light_position", n);
	}
	if (1) {


		//static Vec3 object_color = { 0.5f, 0.5f, 0.5f };
		ImGui::SliderFloat("object_color.r", &object_color.x, 0.0f, 1.0f);
		ImGui::SliderFloat("object_color.g", &object_color.y, 0.0f, 1.0f);
		ImGui::SliderFloat("object_color.b", &object_color.z, 0.0f, 1.0f);
		ImGui::SliderFloat("object_color.w", &object_color.w, 0.0f, 1.0f);
		//static Vec3 light_color = {1.0f, 1.0f, 1.0f};
		ImGui::SliderFloat("light_color.r", &light_color.x, 0.0f, 1.0f);
		ImGui::SliderFloat("light_color.g", &light_color.y, 0.0f, 1.0f);
		ImGui::SliderFloat("light_color.b", &light_color.z, 0.0f, 1.0f);
		app.m_resources.get_shader("light_client").bind();
		app.m_resources.get_shader("light_client").set_uniform("object_color", object_color);
		app.m_resources.get_shader("light_client").set_uniform("light_color", light_color);
		app.m_resources.get_shader("light_client").set_uniform("view_position", app.m_camera.m_position);

		static float specular_strength = 0.5f;
		ImGui::SliderFloat("specular_strength", &specular_strength, 0.0f, 1.0f);
		app.m_resources.get_shader("light_client").set_uniform("specular_strength", specular_strength);

		app.m_resources.get_shader("light_server").bind();
		app.m_resources.get_shader("light_server").set_uniform("light_color", light_color);

	}
	if (0) {
		static auto origin_text = app.m_resources.get_shader("light_client").m_fragment_source.c_str();

		static char* copy = (char*)malloc(strlen(origin_text) + 1);
		strcpy(copy, origin_text);
		static char* text = (char*)origin_text;

		static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
		ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text) + 4000, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
		if (ImGui::Button("Save")) {
			auto& shader = app.m_resources.get_shader("light_client");
			//shader.;
			shader.m_vertex_shader.set_source(shader.m_vertex_source);
			shader.m_vertex_shader.compile();

			shader.m_fragment_shader.set_source(text);
			shader.m_fragment_shader.compile();

			shader.m_program.attach(shader.m_vertex_shader);
			shader.m_program.attach(shader.m_fragment_shader);
			shader.m_program.link();
			shader.m_program.validate();

			shader.m_program.detach(shader.m_vertex_shader);
			shader.m_program.detach(shader.m_fragment_shader);
			shader.update_uniforms();
		}
	}



}
//**************************************************************
//JadeFrame
//**************************************************************
JadeFrame* JadeFrame::m_singleton = nullptr;
auto JadeFrame::get_singleton() -> JadeFrame* {
	return m_singleton;
}
JadeFrame::JadeFrame() {
	std::cout << "JadeFrame is starting..." << std::endl;
	if (m_singleton == nullptr) {
		m_singleton = this;
		m_system_manager.initialize();
		m_time_manager.initialize();
		//m_input_manager.initialize();

		m_system_manager.log();
	} else {
		__debugbreak();
	}
}
auto JadeFrame::run() -> void {
	m_current_app = m_apps[0];
	m_apps.back()->start();
}
auto JadeFrame::add(BaseApp* app) -> void {
	m_apps.push_back(app);
}
//**************************************************************
//~JadeFrame
//**************************************************************

//**************************************************************
//BaseApp
//**************************************************************
BaseApp::BaseApp(const std::string& title, const Vec2& size, const Vec2& position) {
	m_windows[0].initialize(title, size, position);
	//m_windows[1].initialize(title, size, position);
	m_current_window_p = &m_windows[0];

	HWND& win_handle = m_windows[0].m_window_handle;
	HDC& win_device_context = m_windows[0].m_device_context;
	HGLRC& win_render_context = m_windows[0].m_render_context;
	m_renderer.gl_context = GLContext(win_handle, win_device_context, win_render_context);
}
struct Time {
	double current = 0;
	double previous = 0;
	double update = 0;
	double draw = 0;
	double frame = 0;
	double target = 0;
};
auto Time_calc_elapsed(Time& time) -> void {
	time.current = JadeFrame::get_singleton()->m_time_manager.get_time();
	time.update = time.current - time.previous;
	time.previous = time.current;
}
auto Time_frame_control(Time& time) -> void {
	// Frame time control system
	time.current = JadeFrame::get_singleton()->m_time_manager.get_time();
	time.draw = time.current - time.previous;
	time.previous = time.current;

	time.frame = time.update + time.draw;

	if (time.frame < time.target) {
		::Sleep((unsigned int)(float(time.target - time.frame) * 1000.0f));
		time.current = JadeFrame::get_singleton()->m_time_manager.get_time();
		double time_wait = time.current - time.previous;
		time.previous = time.current;
		time.frame += time_wait;
	}
	//__debugbreak();
}
auto BaseApp::start() -> void {
	this->on_init();
	GUI_init(m_current_window_p->m_window_handle);

	Time time;
	time.target = 1 / (double)60;
	while (m_is_running) {
		this->on_update();

		if (m_current_window_p->m_window_state != Windows_Window::WINDOW_STATE::MINIMIZED) {
			Time_calc_elapsed(time);

			m_renderer.swap_buffer(m_current_window_p->m_window_handle);
			m_renderer.clear(m_renderer.gl_context.gl_cache.clear_bitfield);
			GUI_new_frame();
			this->on_draw();
			GUI_render();

			Time_frame_control(time);
		}
		this->poll_events();
	}
}
auto BaseApp::poll_events() -> void {
	JadeFrame::get_singleton()->m_input_manager.handle_input();

	MSG message;
	while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
		if (message.message == WM_QUIT) {
			this->m_is_running = false;
			return;
		}
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}
//**************************************************************
//~BaseApp
//**************************************************************

/*
	********************
	*
	*
	CLIENT PLACEHOLDER
	*
	*
	********************
*/

#if 1 //Test
TestApp::TestApp(const std::string& title, const Vec2& size, const Vec2& position)
	: BaseApp(title, size, position) {
}
auto add_arrows(std::vector<Object>& objects, Material& material) -> void {
	constexpr Color color_light_red = { 1.0f, 0.0f, 0.0f, 1.0f };
	constexpr Color color_light_green = { 0.0f, 1.0f, 0.0f, 1.0f };
	constexpr Color color_light_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	constexpr Color color_dark_red = { 0.2f, 0.0f, 0.0f, 1.0f };
	constexpr Color color_dark_green = { 0.0f, 0.2f, 0.0f, 1.0f };
	constexpr Color color_dark_blue = { 0.0f, 0.0f, 0.2f, 1.0f };

	Mesh* mesh_arrows = new Mesh[6]; // x, y, z
	mesh_arrows[0].current_color = color_light_red;
	mesh_arrows[0].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 10000.0f, .1f, .1f }));
	mesh_arrows[1].current_color = color_light_green;
	mesh_arrows[1].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, 10000.0f, .1f }));
	mesh_arrows[2].current_color = color_light_blue;
	mesh_arrows[2].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, .1f, 10000.0f }));

	mesh_arrows[3].current_color = color_dark_red;
	mesh_arrows[3].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { -10000.0f, .1f, .1f }));
	mesh_arrows[4].current_color = color_dark_green;
	mesh_arrows[4].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, -10000.0f, .1f }));
	mesh_arrows[5].current_color = color_dark_blue;
	mesh_arrows[5].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, .1f, -10000.0f }));

	for (int i = 0; i < 6; i++) {
		Object temp_obj;
		temp_obj.m_transform = Mat4::translate({ 0.0f, 0.0f, 0.0f });
		temp_obj.m_mesh = &mesh_arrows[i];
		temp_obj.m_material = &material;
		temp_obj.m_vertex_array.finalize(*temp_obj.m_mesh);
		objects.push_back(std::move(temp_obj));
	}
}

auto TestApp::on_init() -> void {
	//glEnable(GL_CULL_FACE);
	// Set Up Camera
	if (true) {
		m_camera.perspective(
			{ -20, 10, -5 },
			to_radians(45.0f),
			m_current_window_p->m_size.x / m_current_window_p->m_size.y,
			0.1f,
			10000.0f
		);
	} else {
		//TODO: fix the orthographic camera
		m_camera.othographic(-10, m_windows[0].m_size.x, -10, m_windows[0].m_size.y, -10, 10);
		//m_camera.othographic(-20, 20, -20, 20, -10, 10);
	}

	// Load Resources
	{
		// Load Shaders
		m_resources.set_shader("flat_shader_0", OpenGL_Shader("flat_0"));

		m_resources.set_shader("light_client", OpenGL_Shader("light_client"));
		m_resources.get_shader("light_client").bind();
		m_resources.get_shader("light_client").set_uniform("object_color", object_color);
		m_resources.get_shader("light_client").set_uniform("light_color", light_color);

		m_resources.set_shader("light_server", OpenGL_Shader("light_server"));

		m_resources.set_shader("with_texture_0", OpenGL_Shader("with_texture_0"));

		// Load Textures

		const char* wall_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\wall.jpg";
		const char* smiley_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\awesomeface.png";
		const char* container_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\container.jpg";
		m_resources.set_texture("wall", GLTextureLoader::load(wall_picture_path, GL_TEXTURE_2D, GL_RGB));
		m_resources.set_texture("smiley", GLTextureLoader::load(smiley_picture_path, GL_TEXTURE_2D, GL_RGB));
		m_resources.set_texture("container", GLTextureLoader::load(container_picture_path, GL_TEXTURE_2D, GL_RGB));


		// Load Materials
		m_resources.set_material("coordinate_arrow_material", "flat_shader_0", "wall");
		m_resources.set_material("mat_0", "with_texture_0", "container");

		m_resources.get_shader("with_texture_0").bind();
		m_resources.get_shader("with_texture_0").set_uniform("texture_0", 0);


		m_resources.set_material("light_client", "light_client", "wall");
		m_resources.set_material("light_server", "light_server", "wall");
	}


	// Load Meshes
	{
		Mesh cube_mesh_0;
		cube_mesh_0.current_color = { 0.0, 0.0, 0.0, 0.0 };
		cube_mesh_0.add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
		m_meshes.push_back(cube_mesh_0);

		Mesh light_cube_mesh;
		light_cube_mesh.current_color = { 0.0, 0.0, 0.0, 0.0 };
		light_cube_mesh.add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
		m_meshes.push_back(light_cube_mesh);
	}
	// Create Objects
	Object cube;
	cube.m_transform = Mat4::scale({ 2.0f, 2.0f, 2.0f });
	cube.m_mesh = &m_meshes[0];
	cube.m_material = &m_resources.get_material("mat_0");//light_client

	cube.m_vertex_array.finalize(*cube.m_mesh);
	m_objs.push_back(std::move(cube));

	Object light_cube;
	light_cube.m_transform = Mat4::translate({ 5.0f, -5.0f, -5.0f });
	light_cube.m_mesh = &m_meshes[1];
	light_cube.m_material = &m_resources.get_material("light_server");
	light_cube.m_vertex_array.finalize(*light_cube.m_mesh);
	m_objs.push_back(std::move(light_cube));
	auto vec = Vec3(m_objs[1].m_transform * Vec4(m_objs[1].m_mesh->m_positions[0], 1.0f));
	m_resources.get_shader("light_client").bind();
	m_resources.get_shader("light_client").set_uniform("light_position", vec);

	add_arrows(m_objs, m_resources.get_material("coordinate_arrow_material"));
}
auto TestApp::on_update() -> void {
	m_camera.control();

}


auto TestApp::on_draw() -> void {

	for (size_t i = 0; i < m_objs.size(); i++) {
		m_renderer.push_to_renderer(m_objs[i]);
	}

	const Mat4 view_projection = m_camera.get_view_matrix() * m_camera.get_projection_matrix();
	m_renderer.render_pushed(view_projection);
	draw_GUI(*this);
}
#endif