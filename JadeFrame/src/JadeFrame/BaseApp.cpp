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

		renderer.camera.ortho(
			0.0f, (float)window.width,
			(float)window.height, 0.0f,
			-1.0f, 1.0f
		);

		static Vec2 pos = { 40.0f, 80.0f };

		if(input.isKeyReleased(KEY::D)) pos.x += 10.0f;
		if(input.isKeyReleased(KEY::A)) pos.x -= 10.0f;
		if(input.isKeyReleased(KEY::W)) pos.y -= 10.0f;
		if(input.isKeyReleased(KEY::S)) pos.y += 10.0f;
		pos = input.getMousePosition();
		if(input.isMouseInside()) {
			std::cout << input.getMouseX() << ", " << input.getMouseY() << std::endl;
		}
		RectangleMesh(pos, { 20.0f, 20.0f }).draw();


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
