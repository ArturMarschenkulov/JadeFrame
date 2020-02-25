#include "BaseApp.h"
#include <iostream>
#include "GUI.h"
#include "math/Math.h"

BaseApp* BaseApp::instance = nullptr;
BaseApp::BaseApp() {}

BaseApp::~BaseApp() {
	glfwTerminate();
}

void BaseApp::init_app(const std::string& title, float width, float height) {
	instance = this;
	m_window.init(title, width, height);
	m_shader.init();
	m_renderer.init(&m_shader);

	m_input.setWindowInstance(m_window.get_handle());

	glfwSetMouseButtonCallback(m_window.get_handle(), m_input.mouse_button_callback);
	glfwSetCursorEnterCallback(m_window.get_handle(), m_input.cursor_enter_callback);
	glfwSetKeyCallback(m_window.get_handle(), m_input.key_callback);
}
static void process_input(GLFWwindow* window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void BaseApp::run_app() {
	GUI_init(get_window().get_handle());

	{
		std::cout << "Uniforms: ";
		for(auto uniform : m_shader.m_uniforms) {
			std::cout << uniform.name << " ";
		} std::cout << std::endl;
		std::cout << "Attribs : ";
		for(auto attributes : m_shader.m_attributes) {
			std::cout << attributes.name << " ";
		} std::cout << std::endl;
	}



	//m_renderer.ortho(
	//	0.0f, m_window.get_width(),
	//	m_window.get_height(), 0.0f,
	//	-1.0f, 1.0f
	//);

	//m_renderer.ortho(
	//	0.0f, 100,
	//	100, 0.0f,
	//	-1.0f, 100.0f
	//);

	m_renderer.perspective(
		toRadian(45.0f),
		m_window.get_width() / m_window.get_height(),
		0.1f,
		100.0f
	);
	glEnable(GL_DEPTH_TEST);
	while(!glfwWindowShouldClose(m_window.get_handle())) {


		m_renderer.set_clear_color({ 0.2f, 0.3f, 0.3f, 1.0f });
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		this->poll_events();

		GUI_new_frame();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		m_renderer.start();


		//RectangleMesh({ 50, 50 }, { 50, 50 }).send_to_buffer();

		//MeshManager::make_rectangle({ 50, 50 }, { 50, 50 }).send_to_buffer();
		//MeshManager::make_circle({ 150, 150 }, 30, 20).send_to_buffer();
		//MeshManager::make_triangle({ 100, 100 }, { 200, 100 }, { 100, 200 }).send_to_buffer();

		static Vec3 trans = { 0, 0, 30 };
		ImGui::SliderFloat("transX", &trans.x, -100, 100);
		ImGui::SliderFloat("transY", &trans.y, -100, 100);
		ImGui::SliderFloat("transZ", &trans.z, -100, 100);
		m_renderer.matrix_stack.view_matrix = m_renderer.matrix_stack.view_matrix * Mat4::translate({ 0, 0, trans.z });
		m_renderer.matrix_stack.view_matrix = m_renderer.matrix_stack.view_matrix * Mat4::translate({ 0, trans.y, 0 });
		m_renderer.matrix_stack.view_matrix = m_renderer.matrix_stack.view_matrix * Mat4::translate({ trans.x, 0, 0 });

		//static Vec3 lookat = { 0, 0, 30 };
		//ImGui::SliderFloat("tranX", &lookat.x, -100, 100);
		//ImGui::SliderFloat("tranY", &lookat.y, -100, 100);
		//ImGui::SliderFloat("tranZ", &lookat.z, -100, 100);
		//auto l = Mat4::lookAt({ 0.0f, 0.0, 1.0f }, { 0.0,0,0 }, { 0,0,1 });
		//m_renderer.matrix_stack.view_matrix = m_renderer.matrix_stack.view_matrix * l;

		//m_renderer.matrix_stack.view_matrix = Mat4::translate(trans);
		//MeshManager::make_cube({ 000, 000, 1 }, { 400, 400, 1 }).send_to_buffer();

		static int l = 50;
		ImGui::SliderInt("l", &l, 0, 50);
		{
			m_renderer.set_color({ 1.0f, 0.0, 0.0f, 1.0f }); MeshManager::make_cube({ 0, 0, 0 }, { (float)l, 0.1, 0.1 }).send_to_buffer();
			m_renderer.set_color({ 0.0f, 1.0, 0.0f, 1.0f }); MeshManager::make_cube({ 0, 0, 0 }, { 0.1, (float)l, 0.1 }).send_to_buffer();
			m_renderer.set_color({ 0.0f, 0.0, 1.0f, 1.0f }); MeshManager::make_cube({ 0, 0, 0 }, { 0.1, 0.1, (float)l }).send_to_buffer();

			m_renderer.set_color({ 0.5f, 0.0, 0.0f, 1.0f }); MeshManager::make_cube({ 0, 0, 0 }, { -(float)l, 0.1, 0.1 }).send_to_buffer();
			m_renderer.set_color({ 0.0f, 0.5, 0.0f, 1.0f }); MeshManager::make_cube({ 0, 0, 0 }, { 0.1, -(float)l, 0.1 }).send_to_buffer();
			m_renderer.set_color({ 0.0f, 0.0, 0.5f, 1.0f }); MeshManager::make_cube({ 0, 0, 0 }, { 0.1, 0.1, -(float)l }).send_to_buffer();
		}
		m_renderer.end();


		ImGui::Text("DrawCalls: %d", m_renderer.num_draw_calls);
		GUI_render();

		glfwSwapBuffers(m_window.get_handle());
	}
}
void TimeManager::handle_time() {
	// Frame time control system
	current_time = glfwGetTime();
	draw_time = current_time - previous_time;
	previous_time = current_time;

	frame_time = update_time + draw_time;

	// Wait for some milliseconds...
	if(frame_time < target_time) {
		Sleep((unsigned int)((float)(target_time - frame_time) * 1000.0f));
		current_time = glfwGetTime();
		double extra_time = current_time - previous_time;
		previous_time = current_time;
		frame_time += extra_time;
	}
}

void BaseApp::poll_events() {

	process_input(m_window.get_handle());
	m_time_manager.handle_time();
	m_input.handle_input();
	glfwPollEvents();
}
