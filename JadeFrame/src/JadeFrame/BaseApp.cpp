#include "BaseApp.h"
#include <iostream>
#include "GUI.h"
#include "math/Math.h"
#include <stdint.h>
#include <Windows.h>
#include "FontManager.h"
#include "graphics/GLTexture.h"
#include <time.h>
#include "uilts/Utils.h"

static auto draw_GUI(Camera& camera) -> void {
	ImGui::BeginMainMenuBar();

	ImGui::EndMainMenuBar();


	static bool show = true;
	ImGui::ShowDemoWindow(&show);


	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//ImGui::Checkbox("Depth Test", &b_depth_test);

	//auto& m_renderer = BaseApp::get_instance()->m_renderer;

	ImGui::SliderFloat("cam.m_position.x", &camera.m_position.x, -30, 30);
	ImGui::SliderFloat("cam.m_position.y", &camera.m_position.y, -30, 30);
	ImGui::SliderFloat("cam.m_position.z", &camera.m_position.z, -30, 30);

	ImGui::SliderFloat("cam.m_right.x", &camera.m_right.x, -30, 30);
	ImGui::SliderFloat("cam.m_right.y", &camera.m_right.y, -30, 30);
	ImGui::SliderFloat("cam.m_right.z", &camera.m_right.z, -30, 30);

	ImGui::SliderFloat("cam.m_fovy", &camera.m_fovy, to_radians(-60), to_radians(60));
	static float point_size = 1;
	ImGui::SliderFloat("point size", &point_size, 0, 100);
	glPointSize(point_size);

	static float line_width = 1;
	ImGui::SliderFloat("line width", &line_width, 0, 100);
	glLineWidth(line_width);


}


BaseApp* BaseApp::instance = nullptr;
BaseApp::BaseApp(const std::string& title, Vec2 size, Vec2 position) {
	instance = this;
	m_window = WinWindow::init(title, size, position);
}
BaseApp::~BaseApp() {}


auto BaseApp::start() -> void {
	init();
	GUI_init(m_window.m_window_handle);
	while (m_is_running) {
		float time = (float)m_time_manager.get_time();
		Timestep timstep = time - m_last_frame_time;
		m_last_frame_time = time;
		update();
		if (m_window.is_minimized == false) {
			GUI_new_frame();
			draw();
			GUI_render();
		}
	}
}
auto BaseApp::poll_events() -> void {
	m_input_manager.handle_input();

	MSG message;
	while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
		if (message.message == WM_QUIT) {
			m_is_running = false;
			return;
		}
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}
auto BaseApp::clear(GLbitfield bitfield) -> void {
	glClear(bitfield);
}

auto BaseApp::get_instance() -> BaseApp* {
	return instance;
}

/*
	CLIENT PLACEHOLDER
*/
TestApp::TestApp(const std::string& title, Vec2 size, Vec2 position)
	: BaseApp(title, size, position) {

}

auto TestApp::init() -> void {
	//Renderer
	m_renderer.gl_cache.set_default();
	//~Renderer

	//Camera
	m_camera.perspective(
		{ -20, 10, -5 },
		to_radians(45.0f),
		m_window.m_size.x / m_window.m_size.y,
		0.1f,
		100.0f
	);
	m_renderer.matrix_stack.projection_matrix = m_camera.get_projection_matrix();
	//~Camera

	//Shader
	m_shader.init();
	m_shader.use();
	//Textures
	m_textures[0] = GLTexture::load("C:\\DEV\\Projects\\JadeFrame\\Release\\wall.jpg", GL_TEXTURE_2D, GL_RGB);
	m_textures[1] = GLTexture::load("C:\\DEV\\Projects\\JadeFrame\\Release\\awesomeface.png", GL_TEXTURE_2D, GL_RGB);
	m_shader.set_uniform("texture_0", m_textures[0].m_ID);
	//~Shader

	Color color_green = { 0.0f, 1.0f, 0.0f, 1.0f };

	m_meshes[0].set_color(color_green);
	m_meshes[0].add_to_data(VertexDataFactory::make_cube({ 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }));

	m_meshes[1].set_color(color_green);
	m_meshes[1].add_to_data(VertexDataFactory::make_rectangle({ 0.0f, 0.0f, 0.0f }, { 10.0f, 10.0f, 10.0f }));

	for (int i = 0; i < 10; i++) {
		Object temp_obj;
		temp_obj.m_transform = Mat4::translate({ 1.0f * i, 1.0f * i, 1.0f * i });
		temp_obj.m_mesh = &m_meshes[0];
		temp_obj.m_buffer_data.finalize(*temp_obj.m_mesh);
		temp_obj.m_shader = &m_shader;
		temp_obj.m_texture = &m_textures[0];
		m_objs.push_back(std::move(temp_obj));
	}

	Object m_obj;
	m_obj.m_transform = Mat4::translate({ -1.0f, 0.0f, 0.0f });
	m_obj.m_mesh = &m_meshes[1];
	m_obj.m_buffer_data.finalize(*m_obj.m_mesh);
	m_obj.m_shader = &m_shader;
	m_obj.m_texture = &m_textures[0];
	m_objs.push_back(std::move(m_obj));

	Object m_obj1;
	m_obj1.m_transform = Mat4::translate({ -1.0f, 0.0f, 2.0f });
	m_obj1.m_mesh = &m_meshes[1];
	m_obj1.m_buffer_data.finalize(*m_obj1.m_mesh);
	m_obj1.m_shader = &m_shader;
	m_obj1.m_texture = &m_textures[1];
	m_objs.push_back(std::move(m_obj1));
}
auto TestApp::update() -> void {
	HDC device_context = GetDC(m_window.m_window_handle);

	SwapBuffers(device_context);
	this->clear(m_renderer.gl_cache.clear_bitfield);
	this->poll_events();
}
auto TestApp::draw() -> void {
	m_shader.use();
	m_camera.move();
	{
		m_renderer.matrix_stack.view_matrix = m_camera.get_view_matrix();
		Mat4 view_projection = m_renderer.matrix_stack.view_matrix * m_renderer.matrix_stack.projection_matrix;
		m_shader.set_uniform_matrix("view_projection", view_projection);

		for (size_t i = 0; i < m_objs.size(); i++) {
			m_objs[i].draw();
		}
	}
	draw_GUI(m_camera);
}

struct Layer {
	virtual void on_attach() {
	}
	virtual void on_detach() {
	}
	virtual void on_update(Timestep ts) {
	}
	virtual void on_ImGui_render() {
	}
	virtual void on_event(Event& event) {
	}
};
struct LayerStack {
	std::vector<Layer*> m_layers;
	uint32_t m_layer_insert_index = 0;
	LayerStack() = default;
	~LayerStack() {
		for(Layer* layer : m_layers) {
			layer->on_detach();
			delete layer;
		}
	}
	auto push_layer(Layer* layer) -> void {
		m_layers.emplace(m_layers.begin() + m_layer_insert_index, layer);
		m_layer_insert_index++;
	}

	auto push_overlay(Layer* overlay) -> void {
		m_layers.emplace_back(overlay);
	}
	auto pop_layer(Layer* layer) -> void {
		auto it = std::find(m_layers.begin(), m_layers.begin() + m_layer_insert_index, layer);
		if(it != m_layers.begin() + m_layer_insert_index) {
			layer->on_detach();
			m_layers.erase(it);
			m_layer_insert_index--;
		}
	}
	auto pop_overlay(Layer* overlay) -> void {
		auto it = std::find(m_layers.begin() + m_layer_insert_index, m_layers.end(), overlay);
		if(it != m_layers.end()) {
			overlay->on_detach();
			m_layers.erase(it);
		}
	}


};