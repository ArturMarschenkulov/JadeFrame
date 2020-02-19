#include "BaseApp.h"
#include <iostream>
#include "GUI.h"

BaseApp* BaseApp::instance = nullptr;
BaseApp::BaseApp() {}

BaseApp::~BaseApp() {
	glfwTerminate();
}

void BaseApp::initApp(const std::string& title, float width, float height) {
	instance = this;
	window.init(title, width, height);
	shader.init();
	renderer.init(&shader);

	input.setWindowInstance(window.handle);

	glfwSetMouseButtonCallback(window.handle, input.mouseButtonCallback);
	glfwSetCursorEnterCallback(window.handle, input.cursorEnterCallback);
	glfwSetKeyCallback(window.handle, input.keyCallback);
}
static void processInput(GLFWwindow* window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}



void BaseApp::runApp() {
	GUIinit(window.handle);

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


		renderer.setClearColor({ 0.2f, 0.3f, 0.3f, 1.0f });
		glClear(GL_COLOR_BUFFER_BIT);
		this->pollEvents();

		GUInewFrame();
		renderer.start();

		renderer.ortho(
			0.0f, (float)window.width,
			(float)window.height, 0.0f,
			-1.0f, 1.0f
		);


		renderer.setColor({ 0.0f, 1.0f, 1.0f, 1.0f });//TURQUOISE
		RectangleMesh({ 50.0f, 50.0f }, { 500.0f, 500.0f }).draw();

		renderer.pushMatrix();
			static float tr0 = 70.0f;
			static float tr1 = 10.0f;
			ImGui::SliderFloat("tr0", &tr0, 0, 100);
			ImGui::SliderFloat("tr1", &tr1, 0, 100);
			renderer.translate(tr0, tr1, 0);

			static float sc0 = 1.0f;
			static float sc1 = 1.0f;
			ImGui::SliderFloat("sc0", &sc0, 1, 5);
			ImGui::SliderFloat("sc1", &sc1, 1, 5);

			renderer.translate(100, 100, 0);
			renderer.scale(sc0, sc1, 0);
			renderer.translate(-100, -100, 0);


			renderer.setColor({ 1.0f, 0.0f, 0.0f, 1.0f });//RED
			RectangleMesh({ 100.0f, 100.0f }, { 100.0f, 100.0f }).draw();
		renderer.popMatrix();


		renderer.setColor({ 0.0f, 0.0f, 1.0f, 1.0f });//BLUE
		RectangleMesh({ 300.0f, 100.0f }, { 100.0f, 100.0f }).draw();






		renderer.end();

		GUIrender();

		glfwSwapBuffers(window.handle);
	}
}

void BaseApp::pollEvents() {

	processInput(window.handle);
	renderer.timeManager.handleTime();
	input.handleInput();
	glfwPollEvents();
}
