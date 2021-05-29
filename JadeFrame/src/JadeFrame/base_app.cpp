#include "base_app.h"
#include <utility>



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
		//m_time_manager.initialize();
		//m_input_manager.initialize();

		m_system_manager.log();
	} else {
		__debugbreak();
	}
}
auto JadeFrame::run() -> void {
	m_current_app_p = m_apps[0];
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
#include "gui.h"
BaseApp::BaseApp(const std::string& title, const Vec2& size, const Vec2& position) {
	m_time_manager.initialize();
	m_windows.try_emplace(0, title, size, position);
	//m_windows.try_emplace(1, title, size, position);

	m_current_window_p = &m_windows[0];
	m_main_window_p = &m_windows[0];

	m_renderer.set_context(m_windows[0].m_window_handle);


}
auto BaseApp::start() -> void {


	this->on_init();
	GUI_init(m_current_window_p->m_window_handle);
	m_vulkan_renderer.set_context(m_windows[0].m_window_handle);
	m_time_manager.set_FPS(60);
	while (m_is_running) {

		this->on_update();
		if (m_current_window_p->m_window_state != Windows_Window::WINDOW_STATE::MINIMIZED) {
			m_time_manager.calc_elapsed();
			m_renderer.swap_buffer(m_current_window_p->m_window_handle);
			m_renderer.clear_background();
			GUI_new_frame();



			this->on_draw();
			const Matrix4x4& view_projection = m_camera.get_view_projection_matrix();
			m_renderer.render_pushed(view_projection);

			GUI_render();

			m_time_manager.frame_control();
		}
		this->poll_events();
	}
}
auto BaseApp::poll_events() -> void {
	JadeFrame::get_singleton()->m_input_manager.handle_input();

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

/*
	********************
	*
	*
	CLIENT PLACEHOLDER
	*
	*
	********************
*/

//#include "math/math.h"
//
//static Vec4 s_object_color = { 0.5f, 0.5f, 0.5f, 1.0f };
//static Vec3 s_light_color = { 1.0f, 1.0f, 1.0f };
//static auto draw_GUI(TestApp& app) -> void;
//
//#if 1 //Test
//namespace Test1 {
//TestApp0::TestApp0(const std::string& title, const Vec2& size, const Vec2& position)
//	: BaseApp(title, size, position) {
//	m_current_app = reinterpret_cast<TestApp0*>(this);
//}
//auto add_arrows(std::vector<Object>& objects, Material& material) -> void {
//	constexpr Color color_light_red = { 1.0f, 0.0f, 0.0f, 1.0f };
//	constexpr Color color_light_green = { 0.0f, 1.0f, 0.0f, 1.0f };
//	constexpr Color color_light_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
//	constexpr Color color_dark_red = { 0.2f, 0.0f, 0.0f, 1.0f };
//	constexpr Color color_dark_green = { 0.0f, 0.2f, 0.0f, 1.0f };
//	constexpr Color color_dark_blue = { 0.0f, 0.0f, 0.2f, 1.0f };
//
//	Mesh* mesh_arrows = new Mesh[6]; // x, y, z
//	mesh_arrows[0].current_color = color_light_red;
//	mesh_arrows[0].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 10000.0f, .1f, .1f }));
//	mesh_arrows[1].current_color = color_light_green;
//	mesh_arrows[1].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, 10000.0f, .1f }));
//	mesh_arrows[2].current_color = color_light_blue;
//	mesh_arrows[2].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, .1f, 10000.0f }));
//
//	mesh_arrows[3].current_color = color_dark_red;
//	mesh_arrows[3].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { -10000.0f, .1f, .1f }));
//	mesh_arrows[4].current_color = color_dark_green;
//	mesh_arrows[4].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, -10000.0f, .1f }));
//	mesh_arrows[5].current_color = color_dark_blue;
//	mesh_arrows[5].add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { .1f, .1f, -10000.0f }));
//
//	for (int i = 0; i < 6; i++) {
//		Object temp_obj;
//		temp_obj.m_transform = Matrix4x4::translation_matrix({ 0.0f, 0.0f, 0.0f });
//		temp_obj.m_mesh = &mesh_arrows[i];
//		temp_obj.m_material = &material;
//		temp_obj.m_vertex_array.finalize(*temp_obj.m_mesh);
//		objects.push_back(std::move(temp_obj));
//	}
//}
//
//auto TestApp0::on_init() -> void {
//	// Set Up Camera
//	if (false) {
//		m_camera.perspective_mode(
//			Vec3(0.0f, 0.0f, 3.0f),//{ -20, 10, -5 },
//			to_radians(45.0f),
//			m_current_window_p->m_size.x / m_current_window_p->m_size.y,
//			0.1f,
//			10000.0f
//		);
//	} else {
//		m_camera.othographic_mode(0, m_windows[0].m_size.x, 0, m_windows[0].m_size.y, -100, 100);
//	}
//	// Load Resources
//	if(0)
//	{
//		// Load Shaders
//		m_resources.set_shader("flat_shader_0", OpenGL_Shader("flat_0"));
//
//		m_resources.set_shader("light_client", OpenGL_Shader("light_client"));
//		m_resources.get_shader("light_client").bind();
//		m_resources.get_shader("light_client").set_uniform("object_color", s_object_color);
//		m_resources.get_shader("light_client").set_uniform("light_color", s_light_color);
//
//		m_resources.set_shader("light_server", OpenGL_Shader("light_server"));
//
//		m_resources.set_shader("with_texture_0", OpenGL_Shader("with_texture_0"));
//
//		// Load Textures
//
//		const char* wall_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\wall.jpg";
//		const char* smiley_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\awesomeface.png";
//		const char* container_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\container.jpg";
//		m_resources.set_texture("wall", OpenGL_TextureLoader::load(wall_picture_path, GL_TEXTURE_2D, GL_RGB));
//		m_resources.set_texture("smiley", OpenGL_TextureLoader::load(smiley_picture_path, GL_TEXTURE_2D, GL_RGB));
//		m_resources.set_texture("container", OpenGL_TextureLoader::load(container_picture_path, GL_TEXTURE_2D, GL_RGB));
//
//
//		// Load Materials
//		m_resources.set_material("coordinate_arrow_material", "flat_shader_0", "wall");
//		m_resources.set_material("mat_0", "with_texture_0", "container");
//
//		m_resources.get_shader("with_texture_0").bind();
//		m_resources.get_shader("with_texture_0").set_uniform("texture_0", 0);
//
//
//		m_resources.set_material("light_client", "light_client", "wall");
//		m_resources.set_material("light_server", "light_server", "wall");
//	}
//
//	{
//		m_resources.set_shader("with_texture_0", OpenGL_Shader("with_texture_0"));
//		m_resources.set_shader("flat_shader_0", OpenGL_Shader("flat_0"));
//
//		const char* container_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\container.jpg";
//		const char* wall_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\wall.jpg";
//
//		m_resources.set_texture("container", OpenGL_TextureLoader::load(container_picture_path, GL_TEXTURE_2D, GL_RGB));
//		m_resources.set_texture("wall", OpenGL_TextureLoader::load(wall_picture_path, GL_TEXTURE_2D, GL_RGB));
//
//		m_resources.set_material("mat_0", "with_texture_0", "container");
//		m_resources.set_material("coordinate_arrow_material", "flat_shader_0", "wall");
//	}
//
//	// Load Meshes
//	{
//		Mesh cube_mesh_0;
//		cube_mesh_0.add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
//		m_meshes.push_back(cube_mesh_0);
//
//		Mesh light_cube_mesh;
//		light_cube_mesh.add_to_data(VertexDataFactory::make_cube({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
//		m_meshes.push_back(light_cube_mesh);
//
//		Mesh rect_mesh;
//		rect_mesh.add_to_data(VertexDataFactory::make_rectangle({ 0.0f, 0.0f, 0.0f }, { 100.0f, 100.0f, 0.0f }));
//		m_meshes.push_back(rect_mesh);
//	}
//	{
//		const int i_num = 13;
//		const int j_num = 8;
//		for (int i = 0; i < i_num; i++) {
//			for (int j = 0; j < j_num; j++) {
//				if ((i + j) % 2) {
//					Object plane;
//					plane.m_transform = Matrix4x4::translation_matrix({ i * 50.0f, j * 50.0f, 0.0f });
//					plane.m_mesh = &m_meshes[2];
//					plane.m_material = &m_resources.get_material("mat_0");
//					plane.m_vertex_array.finalize(*plane.m_mesh);
//					m_objs.push_back(std::move(plane));
//				}
//			}
//		}
//	}
//#if 0
//	{
//		// Create Objects
//		Object cube;
//		cube.m_transform = Matrix4x4::scale_matrix({ 2.0f, 2.0f, 2.0f });
//		cube.m_mesh = &m_meshes[0];
//		cube.m_material = &m_resources.get_material("mat_0");//light_client
//		cube.m_vertex_array.finalize(*cube.m_mesh);
//		m_objs.push_back(std::move(cube));
//		{
//			Object light_cube;
//			light_cube.m_transform = Matrix4x4::translation_matrix({ 5.0f, 5.0f, 5.0f });
//			light_cube.m_mesh = &m_meshes[1];
//			light_cube.m_material = &m_resources.get_material("light_server");
//			light_cube.m_vertex_array.finalize(*light_cube.m_mesh);
//			m_objs.push_back(std::move(light_cube));
//		}
//		{
//			Object light_cube;
//			light_cube.m_transform = Matrix4x4::translation_matrix({ 5.0f, -5.0f, 5.0f });
//			light_cube.m_mesh = &m_meshes[1];
//			light_cube.m_material = &m_resources.get_material("light_server");
//			light_cube.m_vertex_array.finalize(*light_cube.m_mesh);
//			m_objs.push_back(std::move(light_cube));
//		}
//		{
//			Object light_cube;
//			light_cube.m_transform = Matrix4x4::translation_matrix({ -5.0f, -5.0f, 5.0f });
//			light_cube.m_mesh = &m_meshes[1];
//			light_cube.m_material = &m_resources.get_material("light_server");
//			light_cube.m_vertex_array.finalize(*light_cube.m_mesh);
//			m_objs.push_back(std::move(light_cube));
//		}
//		{
//			Object light_cube;
//			light_cube.m_transform = Matrix4x4::translation_matrix({ -5.0f, 5.0f, 5.0f });
//			light_cube.m_mesh = &m_meshes[1];
//			light_cube.m_material = &m_resources.get_material("light_server");
//			light_cube.m_vertex_array.finalize(*light_cube.m_mesh);
//			m_objs.push_back(std::move(light_cube));
//		}
//		auto vec = Vec3(m_objs[1].m_transform * Vec4(m_objs[1].m_mesh->m_positions[0], 1.0f));
//		m_resources.get_shader("light_client").bind();
//		m_resources.get_shader("light_client").set_uniform("light_position", vec);
//	}
//#endif
//	add_arrows(m_objs, m_resources.get_material("coordinate_arrow_material"));
//}
//auto TestApp0::on_update() -> void {
//	m_camera.control();
//	auto& i = JadeFrame::get_singleton()->m_input_manager;
//	const auto& mp = i.get_mouse_position();
//	if (i.is_key_pressed(KEY::K)) {
//		__debugbreak();
//	}
//}
//auto TestApp0::on_draw() -> void {
//
//	for (size_t i = 0; i < m_objs.size(); i++) {
//		m_renderer.push_to_renderer(m_objs[i]);
//	}
//
//	const Matrix4x4 view_projection = m_camera.get_view_matrix() * m_camera.get_projection_matrix();
//	m_renderer.render_pushed(view_projection);
//	m_renderer.m_command_buffer.m_render_commands.clear();
//	//draw_GUI(*this);
//}
//}
//#endif
//
//static auto draw_GUI(TestApp& app) -> void {
//	ImGui::BeginMainMenuBar();
//
//	ImGui::EndMainMenuBar();
//
//
//	static bool show = true;
//	ImGui::ShowDemoWindow(&show);
//
//
//
//	float& FPS_max = app.m_time_manager.max_FPS;
//	ImGui::SliderFloat("max FPS", &FPS_max, 0, 1000);
//	app.m_time_manager.set_FPS(FPS_max);
//
//
//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//
//
//
//
//	Vec3& cam_pos = app.m_camera.m_position;
//	ImGui::SliderFloat3("cam_pos", &cam_pos.x, -100.0f, 100.0);
//
//	float* yp[2];
//	yp[0] = &app.m_camera.m_yaw;
//	yp[1] = &app.m_camera.m_pitch;
//
//	ImGui::SliderFloat2("yaw pitch", *yp, -100.0f, 100.0f);
//}