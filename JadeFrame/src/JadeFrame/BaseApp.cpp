#include "BaseApp.h"
#include <iostream>
#include "GUI.h"

BaseApp* BaseApp::instance = nullptr;
BaseApp::BaseApp() {}

BaseApp::~BaseApp() {
	glfwTerminate();
}

void BaseApp::init_app(const std::string& title, float width, float height) {
	instance = this;
	window.init(title, width, height);
	shader.init();
	renderer.init(&shader);

	input.setWindowInstance(window.handle);

	glfwSetMouseButtonCallback(window.handle, input.mouse_button_callback);
	glfwSetCursorEnterCallback(window.handle, input.cursor_enter_callback);
	glfwSetKeyCallback(window.handle, input.key_callback);
}
static void process_input(GLFWwindow* window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void BaseApp::run_app() {
	GUI_init(window.handle);

	{
		std::cout << "Uniforms: ";
		for(auto uniform : shader.m_uniforms) {
			std::cout << uniform.name << " ";
		} std::cout << std::endl;
		std::cout << "Attribs : ";
		for(auto attributes : shader.m_attributes) {
			std::cout << attributes.name << " ";
		} std::cout << std::endl;
	}





		while(!glfwWindowShouldClose(window.handle)) {


			renderer.set_clear_color({ 0.2f, 0.3f, 0.3f, 1.0f });
			glClear(GL_COLOR_BUFFER_BIT);
			this->poll_events();

			GUI_new_frame();

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			renderer.start();
			renderer.ortho(
				0.0f, (float)window.width,
				(float)window.height, 0.0f,
				-1.0f, 1.0f
			);
			//Vec3 cameraPos = Vec3(0.0f, 0.0f, 3.0f);
			//Vec3 cameraFront = Vec3(0.0f, 0.0f, -1.0f);
			//Vec3 cameraUp = Vec3(0.0f, 1.0f, 0.0f);
			//renderer.perspective(
			//	40.0f * 0.01745,
			//	window.width / window.height,
			//	0.1f, 100.0f
			//);
			//Mat4 view = Mat4::lookAt({ 0.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
			//renderer.matrixStack.viewMatrix = view;


			if(0) {
				renderer.set_color({ 0.0f, 1.0f, 1.0f, 1.0f });//TURQUOISE
				RectangleMesh({ 50.0f, 50.0f }, { 500.0f, 500.0f }).send_to_buffer();

				renderer.push_matrix();
				static float tr0 = 70.0f;
				static float tr1 = 10.0f;
				ImGui::SliderFloat("tr0", &tr0, 0, 100);
				ImGui::SliderFloat("tr1", &tr1, 0, 100);
				renderer.translate(tr0, tr1, 0);

				static float sc0 = 1.0f;
				static float sc1 = 1.0f;
				ImGui::SliderFloat("sc0", &sc0, 1, 5);
				ImGui::SliderFloat("sc1", &sc1, 1, 5);

				renderer.translate(150, 150, 0);
				renderer.scale(sc0, sc1, 0);
				renderer.translate(-150, -150, 0);


				renderer.set_color({ 1.0f, 0.0f, 0.0f, 1.0f });//RED

				RectangleMesh({ 100.0f, 100.0f }, { 100.0f, 100.0f })
					.send_to_buffer();
				renderer.pop_matrix();


				renderer.set_color({ 0.0f, 0.0f, 1.0f, 1.0f });//BLUE
				RectangleMesh({ 300.0f, 100.0f }, { 100.0f, 100.0f }).send_to_buffer();

			}


			static int amount = 1000;
			ImGui::SliderInt("amount0", &amount, 1, 10);
			ImGui::SliderInt("amount1", &amount, 1, 100);
			ImGui::SliderInt("amount2", &amount, 1, 1000);
			ImGui::SliderInt("amount3", &amount, 1, 10000);
			ImGui::SliderInt("amount4", &amount, 1, 100000);
			Vec2 size;
			size.x = window.width / amount;
			size.y = window.height / amount;
			Vec2 pos = { 0 };
			for(int i = 0; i < amount; i++) {
				renderer.set_color({ 0.5, 0.5, 0.5, 1.0f });
				RectangleMesh(pos + size * i, size).send_to_buffer();
			}




			renderer.end();
			ImGui::Text("DrawCalls: %d", renderer.num_draw_calls);
			GUI_render();

			glfwSwapBuffers(window.handle);
		}
}
void TimeManager::handle_time() {
	// Frame time control system
	currentTime = glfwGetTime();
	drawTime = currentTime - previousTime;
	previousTime = currentTime;

	frameTime = updateTime + drawTime;

	// Wait for some milliseconds...
	if(frameTime < targetTime) {
		Sleep((unsigned int)((float)(targetTime - frameTime) * 1000.0f));
		currentTime = glfwGetTime();
		double extraTime = currentTime - previousTime;
		previousTime = currentTime;
		frameTime += extraTime;
	}
}

void BaseApp::poll_events() {

	process_input(window.handle);
	time_manager.handle_time();
	input.handleInput();
	glfwPollEvents();
}
