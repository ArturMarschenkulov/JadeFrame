#pragma once
#include<JadeFrame.h>
#include<JadeFrame/math/math.h>
#include<JadeFrame/gui.h>
#include<JadeFrame/utils/utils.h>


namespace Test1 {

struct Drop {
	Drop() {
		BaseApp* app = JadeFrame::get_singleton()->m_current_app_p;
		const f32 window_width = app->m_main_window_p->m_size.x;

		x = (f32)get_random_number(0, window_width);
		const f32 rando = (f32)get_random_number(1, 30);
		y_speed = map_range(rando, 1, 30, 1, 3);

		obj.m_transform = Matrix4x4::scale_matrix({ 10.0f, 80.0f, 1.0f }) * Matrix4x4::translation_matrix({ x, y, 0.0f });
		app->m_resources.get_mesh("rectangle").set_color({ 138_u8, 43_u8, 226_u8, 255_u8 });
		obj.m_mesh = &app->m_resources.get_mesh("rectangle");
		obj.m_material = &app->m_resources.get_material("flat_color_mat");
		obj.m_vertex_array.finalize(*obj.m_mesh);
	}

	auto fall() -> void {
		y = y + y_speed;
		obj.m_transform = Matrix4x4::scale_matrix({ 10.0f, 80.0f, 1.0f }) * Matrix4x4::translation_matrix({ x, y, 0.0f });
		const f32 window_height = JadeFrame::get_singleton()->m_current_app_p->m_main_window_p->m_size.y;
		if (y >= window_height) {
			y = -100;
		}
	}
	auto show() const -> void {
		Renderer& renderer = JadeFrame::get_singleton()->m_apps[0]->m_renderer;
		renderer.push_to_renderer(obj);
	}

	Mesh mesh;
	Object obj = {};
	f32 x = 10;
	f32 y = 10;
	f32 y_speed = 1;
};

struct Example_0 : public BaseApp {
	Example_0(const std::string& title, const Vec2& size, const Vec2& position = { -1, -1 });
	virtual ~Example_0() = default;

	virtual auto on_init() -> void override;
	virtual auto on_update() -> void override;
	virtual auto on_draw() -> void override;

public:
	std::deque<Mesh> m_meshes;
	std::vector<Object> m_objs;
	std::deque<Drop> drops;
};
}


static auto draw_GUI(Test1::Example_0& app) -> void;

namespace Test1 {
Example_0::Example_0(const std::string& title, const Vec2& size, const Vec2& position)
	: BaseApp(title, size, position) {
	m_current_app_p = reinterpret_cast<Example_0*>(this);
}

auto Example_0::on_init() -> void {
	m_renderer.set_clear_color({ 230_u8, 230_u8, 250_u8, 253_u8 });

	// Set Up Camera
	m_camera.othographic_mode(0, m_windows[0].m_size.x, m_windows[0].m_size.y, 0, -1, 1);

	// Load Resources
	{
		const char* wall_picture_path = "C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\wall.jpg";
		m_resources.set_texture("wall", OpenGL_TextureLoader::load(wall_picture_path, GL_TEXTURE_2D, GL_RGB));

		m_resources.set_shader("flat_shader_0", OpenGL_Shader("flat_0"));

		m_resources.set_material("flat_color_mat", "flat_shader_0", "wall");

		Mesh rectangle_mesh;
		rectangle_mesh.add_to_data(VertexDataFactory::make_rectangle({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }));
		m_resources.set_mesh("rectangle", rectangle_mesh);
	}
	for (u32 i = 0; i < 100; i++) {
		drops.emplace_back();
	}

}
auto Example_0::on_update() -> void {
	m_camera.control();
	for (u32 i = 0; i < drops.size(); i++) {
		drops[i].fall();
	}
	if (JadeFrame::get_singleton()->m_input_manager.is_key_released(KEY::P)) {
		//std::thread t(&Renderer::take_screenshot, &m_renderer);
		m_renderer.take_screenshot("im.png");
	}
}
auto Example_0::on_draw() -> void {

	for (u32 i = 0; i < drops.size(); i++) {
		drops[i].show();
	}



	draw_GUI(*this);
}
}


static auto draw_GUI(Test1::Example_0& app) -> void {
	//ImGui::BeginMainMenuBar();

	//ImGui::EndMainMenuBar();

	//static bool show = true;
	//ImGui::ShowDemoWindow(&show);

	//static f32 posx = 0;
	//static f32 posy = 0;
	//static f32 posz = 0;
	//ImGui::SliderFloat("posx", &posx, 0, app.m_current_window_p->m_size.x);
	//ImGui::SliderFloat("posy", &posy, 0, app.m_current_window_p->m_size.y);
	//ImGui::SliderFloat("posz", &posz, -10, 10);
	//app.m_objs[0].m_transform = Matrix4x4::scale_matrix({ 10.0f, 80.0f, 1.0f }) * Matrix4x4::translation_matrix({ posx,  posy, posz });

	f32& FPS_max = app.m_time_manager.max_FPS;
	ImGui::SliderFloat("max FPS", &FPS_max, 0, 1000);
	app.m_time_manager.set_FPS(FPS_max);


	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	Vec3& cam_pos = app.m_camera.m_position;
	ImGui::SliderFloat3("cam_pos", &cam_pos.x, -100.0f, 100.0);

	f32* yp[2];
	yp[0] = &app.m_camera.m_yaw;
	yp[1] = &app.m_camera.m_pitch;

	ImGui::SliderFloat2("yaw pitch", *yp, -100.0f, 100.0f);
}
using TestApp = Test1::Example_0;