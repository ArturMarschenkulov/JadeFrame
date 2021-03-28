#include "BaseApp.h"

#include "GUI.h"
#include "math/Math.h"

#include "graphics/opengl/GLTexture.h"

#include <stdint.h>
#include <iostream>

static auto draw_GUI(Camera1& camera, Windows_Window* current_window_p) -> void {
	ImGui::BeginMainMenuBar();

	ImGui::EndMainMenuBar();


	static bool show = true;
	ImGui::ShowDemoWindow(&show);


	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//ImGui::Checkbox("Depth Test", &b_depth_test);

	//auto& m_renderer = BaseApp::get_instance()->m_renderer;

	//ImGui::SliderFloat("cam.m_position.x", &camera.m_position.x, -30, 30);
	//ImGui::SliderFloat("cam.m_position.y", &camera.m_position.y, -30, 30);
	//ImGui::SliderFloat("cam.m_position.z", &camera.m_position.z, -30, 30);

	//ImGui::SliderFloat("cam.m_right.x", &camera.m_right.x, -30, 30);
	//ImGui::SliderFloat("cam.m_right.y", &camera.m_right.y, -30, 30);
	//ImGui::SliderFloat("cam.m_right.z", &camera.m_right.z, -30, 30);

	//ImGui::SliderFloat("cam.m_fovy", &camera.m_fovy, to_radians(-60), to_radians(60));
	static float point_size = 1;
	ImGui::SliderFloat("point size", &point_size, 0, 100);
	glPointSize(point_size);

	static float line_width = 1;
	ImGui::SliderFloat("line width", &line_width, 0, 100);
	glLineWidth(line_width);
	//static bool is_v_snyc_on = true;
	//ImGui::Checkbox("VSync", &is_v_snyc_on);

	//static bool ortho = false;
	//ImGui::Checkbox("Ortho", &ortho);
	//if (ortho) {
	//	camera.perspective(
	//		{ -20, 10, -5 },
	//		to_radians(45.0f),
	//		current_window_p->m_size.x / current_window_p->m_size.y,
	//		0.1f,
	//		10000.0f
	//	);
	//} else {
	//	camera.othographic(-10, 10, 10, -10, -10, 10);
	//}
	//BaseApp::get_singleton()->m_current_window_p->set_v_sync(is_v_snyc_on);


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
		Sleep((unsigned int)(float(time.target - time.frame) * 1000.0f));
		time.current = JadeFrame::get_singleton()->m_time_manager.get_time();
		double time_wait = time.current - time.previous;
		time.previous = time.current;
		time.frame += time_wait;
	}
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
	mesh_arrows[0].set_color(color_light_red);
	mesh_arrows[0].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 10000.0f, .1f, .1f }));
	mesh_arrows[1].set_color(color_light_green);
	mesh_arrows[1].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, 10000.0f, .1f }));
	mesh_arrows[2].set_color(color_light_blue);
	mesh_arrows[2].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, .1f, 10000.0f }));

	mesh_arrows[3].set_color(color_dark_red);
	mesh_arrows[3].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { -10000.0f, .1f, .1f }));
	mesh_arrows[4].set_color(color_dark_green);
	mesh_arrows[4].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, -10000.0f, .1f }));
	mesh_arrows[5].set_color(color_dark_blue);
	mesh_arrows[5].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, .1f, -10000.0f }));

	for (int i = 0; i < 6; i++) {
		Object temp_obj;
		temp_obj.m_transform = Mat4::translate({ 0.0f, 0.0f, 0.0f });
		temp_obj.m_mesh = &mesh_arrows[i];
		temp_obj.m_material = &material;
		temp_obj.m_buffer_data.finalize(*temp_obj.m_mesh);
		objects.push_back(std::move(temp_obj));
	}
}

auto TestApp::on_init() -> void {
	glEnable(GL_CULL_FACE);
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
	m_renderer.matrix_stack.projection_matrix = m_camera.get_projection_matrix();
	m_renderer.m_current_camera = &m_camera;

	// Load Resources
	{
		// Load Shaders
		m_resources.set_shader("flat_shader_0", OpenGL_Shader("flat_0"));

		m_resources.set_shader("light_client", OpenGL_Shader("light_client"));
		m_resources.get_shader("light_client").bind();
		m_resources.get_shader("light_client").set_uniform("object_color", Vec3{ 0.5f, 0.5f, 0.5f });
		m_resources.get_shader("light_client").set_uniform("light_color", Vec3{ 1.0f, 1.0f, 1.0f });

		m_resources.set_shader("light_server", OpenGL_Shader("light_server"));

		// Load Textures

		const char* wall_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\wall.jpg";
		const char* smiley_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\awesomeface.png";
		m_resources.set_texture("wall", GLTextureLoader::load(wall_picture_path, GL_TEXTURE_2D, GL_RGB));
		m_resources.set_texture("smiley", GLTextureLoader::load(smiley_picture_path, GL_TEXTURE_2D, GL_RGB));


		// Load Materials
		m_resources.set_material("coordinate_arrow_material", "flat_shader_0", "wall");
		m_resources.set_material("light_client", "light_client", "wall");
		m_resources.set_material("light_server", "light_server", "wall");
	}


	// Load Meshes
	{
		Mesh cube_mesh_0;
		cube_mesh_0.set_color({ 0.0, 0.0, 0.0, 0.0 });
		cube_mesh_0.add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
		m_meshes.push_back(cube_mesh_0);

		Mesh light_cube_mesh;
		light_cube_mesh.set_color({ 0.0, 0.0, 0.0, 0.0 });
		light_cube_mesh.add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
		m_meshes.push_back(light_cube_mesh);
	}
	// Create Objects
	Object cube;
	//cube.m_transform = Mat4::translate({ 0.0f, 0.0f, 0.0f });
	cube.m_transform = Mat4::scale({ 2.0f, 2.0f, 2.0f });
	cube.m_mesh = &m_meshes[0];
	cube.m_material = &m_resources.get_material("light_client");
	cube.m_buffer_data.finalize(*cube.m_mesh);
	m_objs.push_back(std::move(cube));

	Object light_cube;
	light_cube.m_transform = Mat4::translate({ 0.0f, 0.0f, 0.0f });
	light_cube.m_mesh = &m_meshes[1];
	light_cube.m_material = &m_resources.get_material("light_server");
	light_cube.m_buffer_data.finalize(*light_cube.m_mesh);
	m_objs.push_back(std::move(light_cube));

	m_resources.get_shader("light_client").set_uniform("light_position", light_cube.m_mesh->m_positions[0]);
	//add_arrows(m_objs, m_resources.get_material("coordinate_arrow_material"));
}
auto TestApp::on_update() -> void {
	m_camera.control();
}


auto TestApp::on_draw() -> void {
	static Vec3 curr_vec = { 0.0f, 0.0f, 0.0f };
	auto prev_vec = curr_vec;

	ImGui::SliderFloat("vec_x", &curr_vec.x, -30, +30);
	ImGui::SliderFloat("vec_y", &curr_vec.y, -30, +30);
	ImGui::SliderFloat("vec_z", &curr_vec.z, -30, +30);

	if (prev_vec != curr_vec) {
		m_objs[1].m_transform = Mat4::translate(curr_vec);
		m_resources.get_shader("light_client").bind();
		const Vec3 n = Vec3(m_objs[1].m_transform * Vec4(m_objs[1].m_mesh->m_positions[0], 1.0f));
		m_resources.get_shader("light_client").set_uniform("light_position", n);
		auto o = n;
		std::cout << "ll: " << o.x << ", " << o.y << ", " << o.z << std::endl;
	}

	static Vec2 vp_pos = { 0.0f, 0.0f };
	static Vec2 vp_size = { m_current_window_p->m_size };
	auto prev_vp_pos = vp_pos;
	auto prev_vp_size = vp_size;

	ImGui::SliderFloat("vp_pos.x", &vp_pos.x, 0, m_current_window_p->m_position.x);
	ImGui::SliderFloat("vp_pos.y", &vp_pos.y, 0, m_current_window_p->m_position.y);
	ImGui::SliderFloat("vp_size.x", &vp_size.x, 0, m_current_window_p->m_size.x);
	ImGui::SliderFloat("vp_size.y", &vp_size.y, 0, m_current_window_p->m_size.y);

	if((prev_vp_pos != vp_pos) || (prev_vp_size != vp_size)) {
		m_renderer.set_viewport(vp_pos.x, vp_pos.y, vp_size.x, vp_size.y);
		std::cout << "vv: " << std::endl;

	}

	for (size_t i = 0; i < m_objs.size(); i++) {
		m_renderer.render(m_objs[i]);
	}


	draw_GUI(m_camera, m_current_window_p);
}
#endif



#if 0 //Test 2
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
	mesh_arrows[0].set_color(color_light_red);
	mesh_arrows[0].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 10000.0f, .1f, .1f }));
	mesh_arrows[1].set_color(color_light_green);
	mesh_arrows[1].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, 10000.0f, .1f }));
	mesh_arrows[2].set_color(color_light_blue);
	mesh_arrows[2].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, .1f, 10000.0f }));

	mesh_arrows[3].set_color(color_dark_red);
	mesh_arrows[3].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { -10000.0f, .1f, .1f }));
	mesh_arrows[4].set_color(color_dark_green);
	mesh_arrows[4].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, -10000.0f, .1f }));
	mesh_arrows[5].set_color(color_dark_blue);
	mesh_arrows[5].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, .1f, -10000.0f }));

	for (int i = 0; i < 6; i++) {
		Object temp_obj;
		temp_obj.m_transform = Mat4::translate({ 0.0f, 0.0f, 0.0f });
		temp_obj.m_mesh = &mesh_arrows[i];
		temp_obj.m_material = &material;
		temp_obj.m_buffer_data.finalize(*temp_obj.m_mesh);
		objects.push_back(std::move(temp_obj));
	}
}

auto TestApp::init() -> void {
	glEnable(GL_CULL_FACE);
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
	m_renderer.matrix_stack.projection_matrix = m_camera.get_projection_matrix();
	m_renderer.m_current_camera = &m_camera;

	// Load Resources
	{
		// Load Shaders
		m_resources.set_shader("flat_shader_0", GLShader("flat_0"));
		m_resources.set_shader("texture_shader_0", GLShader("with_texture_0"));
		m_resources.set_shader("depth_testing", GLShader("depth_testing_0"));

		m_resources.set_shader("light_client", GLShader("light_client"));
		m_resources.get_shader("light_client").bind();
		m_resources.get_shader("light_client").set_uniform("object_color", Vec3{ 0.5f, 0.5f, 0.5f });
		m_resources.get_shader("light_client").set_uniform("light_color", Vec3{ 1.0f, 1.0f, 1.0f });

		m_resources.set_shader("light_server", GLShader("light_server"));

		// Load Textures

		const char* wall_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\wall.jpg";
		const char* smiley_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\awesomeface.png";
		m_resources.set_texture("wall", GLTextureLoader::load(wall_picture_path, GL_TEXTURE_2D, GL_RGB));
		m_resources.set_texture("smiley", GLTextureLoader::load(smiley_picture_path, GL_TEXTURE_2D, GL_RGB));


		// Load Materials
		m_resources.set_material("mat_0", "texture_shader_0", "wall");
		m_resources.set_material("mat_1", "texture_shader_0", "smiley");
		m_resources.set_material("coordinate_arrow_material", "flat_shader_0", "wall");
		m_resources.set_material("light_client", "light_client", "wall");
		m_resources.set_material("light_server", "light_server", "wall");
	}


	// Load Meshes
	{
		Mesh cube_mesh;
		cube_mesh.set_color({ 0.0, 0.0, 0.0, 0.0 });
		cube_mesh.add_to_data(VertexDataFactory::make_cube({ 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }));
		m_meshes.push_back(cube_mesh);

		Mesh rectangle_mesh;
		rectangle_mesh.set_color({ 0.0, 0.0, 0.0, 0.0 });
		rectangle_mesh.add_to_data(VertexDataFactory::make_rectangle({ 0.0f, 0.0f, 0.0f }, { 10.0f, 10.0f, 10.0f }));
		m_meshes.push_back(rectangle_mesh);

		Mesh cube_mesh_0;
		cube_mesh_0.set_color({ 0.0, 0.0, 0.0, 0.0 });
		cube_mesh_0.add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
		m_meshes.push_back(cube_mesh_0);

		Mesh light_cube_mesh;
		light_cube_mesh.set_color({ 0.0, 0.0, 0.0, 0.0 });
		light_cube_mesh.add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
		m_meshes.push_back(light_cube_mesh);
	}

	Object cube;
	//cube.m_transform = Mat4::translate({ 0.0f, 0.0f, 0.0f });
	cube.m_transform = Mat4::scale({ 2.0f, 2.0f, 2.0f });
	cube.m_mesh = &m_meshes[2];
	cube.m_material = &m_resources.get_material("light_client");
	cube.m_buffer_data.finalize(*cube.m_mesh);
	m_objs.push_back(std::move(cube));

	Object light_cube;
	light_cube.m_transform = Mat4::translate({ 0.0f, 0.0f, 0.0f });
	light_cube.m_mesh = &m_meshes[3];
	light_cube.m_material = &m_resources.get_material("light_server");
	light_cube.m_buffer_data.finalize(*light_cube.m_mesh);
	m_objs.push_back(std::move(light_cube));

	m_resources.get_shader("light_client").set_uniform("light_position", light_cube.m_mesh->m_positions[0]);

	//cube.m_material->m_shader->set_uniform("light_position", light_cube.m_mesh->m_positions[0]);
	// Create Objects
	if (false) {

		// several small cubes
		for (int i = 0; i < 10; i++) {
			Object temp_obj;
			temp_obj.m_transform = Mat4::translate({ 1.0f * i, 1.0f * i, 1.0f * i });
			temp_obj.m_mesh = &m_meshes[0];
			temp_obj.m_material = &m_resources.get_material("mat_0");
			temp_obj.m_buffer_data.finalize(*temp_obj.m_mesh);
			m_objs.push_back(std::move(temp_obj));
		}


		// Plane Wall Texture
		Object m_obj;
		m_obj.m_transform = Mat4::translate({ -1.0f, 0.0f, 0.0f });
		m_obj.m_mesh = &m_meshes[1];
		m_obj.m_material = &m_resources.get_material("mat_0");
		m_obj.m_buffer_data.finalize(*m_obj.m_mesh);
		m_objs.push_back(std::move(m_obj));

		// Plane Smiley Texture
		Object m_obj1;
		m_obj1.m_transform = Mat4::translate({ -1.0f, 0.0f, 2.0f });
		m_obj1.m_mesh = &m_meshes[1];
		m_obj1.m_material = &m_resources.get_material("mat_1");
		m_obj1.m_buffer_data.finalize(*m_obj1.m_mesh);
		m_objs.push_back(std::move(m_obj1));
	}
	add_arrows(m_objs, m_resources.get_material("coordinate_arrow_material"));
}
auto TestApp::update() -> void {

}


auto TestApp::draw() -> void {
	m_camera.control();

	static Vec3 curr_vec = { 0.0f, 0.0f, 0.0f };
	auto prev_vec = curr_vec;

	ImGui::SliderFloat("vec_x", &curr_vec.x, -30, +30);
	ImGui::SliderFloat("vec_y", &curr_vec.y, -30, +30);
	ImGui::SliderFloat("vec_z", &curr_vec.z, -30, +30);

	if (prev_vec != curr_vec) {
		m_objs[1].m_transform = Mat4::translate(curr_vec);
		m_resources.get_shader("light_client").bind();
		const Vec3 n = Vec3(m_objs[1].m_transform * Vec4(m_objs[1].m_mesh->m_positions[0], 1.0f));
		m_resources.get_shader("light_client").set_uniform("light_position", n);
		auto o = n;
		std::cout << "ll: " << o.x << ", " << o.y << ", " << o.z << std::endl;
	}

	static Vec2 vp_pos = { 0.0f, 0.0f };
	static Vec2 vp_size = { m_current_window_p->m_size };
	auto prev_vp_pos = vp_pos;
	auto prev_vp_size = vp_size;

	ImGui::SliderFloat("vp_pos.x", &vp_pos.x, 0, m_current_window_p->m_position.x);
	ImGui::SliderFloat("vp_pos.y", &vp_pos.y, 0, m_current_window_p->m_position.y);
	ImGui::SliderFloat("vp_size.x", &vp_size.x, 0, m_current_window_p->m_size.x);
	ImGui::SliderFloat("vp_size.y", &vp_size.y, 0, m_current_window_p->m_size.y);

	if ((prev_vp_pos != vp_pos) || (prev_vp_size != vp_size)) {
		m_renderer.set_viewport(vp_pos.x, vp_pos.y, vp_size.x, vp_size.y);
		std::cout << "vv: " << std::endl;

	}

	for (size_t i = 0; i < m_objs.size(); i++) {
		m_renderer.render(m_objs[i]);
	}


	draw_GUI(m_camera, m_current_window_p);
}
#endif




