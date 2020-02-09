#include "BaseApp.h"
#include <iostream>

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
	glfwSetKeyCallback(window.handle, input.keyCallback);
}
static void processInput(GLFWwindow* window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}
void BaseApp::runApp() {



	Vec2 pos = { -0.5, +0.5 };
	while(!glfwWindowShouldClose(window.handle)) {
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window.handle);

		renderer.startDraw();

		//int num = 4;
		//Vec2 pos = { -1.0f, +1.0f };
		//for(int i = 0; i < num; i++) {
		//	Vec2 size = { +2.0f/num, -2.0f/num };
		//	renderer.drawRectangle(pos , size);
		//	pos += size;
		//}

		if(input.isKeyDown(KEY::D)) pos.x += 0.1f;
		if(input.isKeyDown(KEY::A)) pos.x -= 0.1f;
		if(input.isKeyDown(KEY::W)) pos.y -= 0.1f;
		if(input.isKeyDown(KEY::S)) pos.y += 0.1f;

		if(input.isKeyDown(KEY::X)) {
			renderer.drawRectangle(pos, { +0.5f, -0.5f });
		}



		renderer.endDraw();

		glfwSwapBuffers(window.handle);
		glfwPollEvents();


	}
}
