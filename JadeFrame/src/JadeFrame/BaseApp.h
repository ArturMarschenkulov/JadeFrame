#pragma once
#include "Window.h"
#include "Renderer.h"

void processInput(GLFWwindow* window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

class BaseApp {
public:
	Window window;
	Shader shader;
	Renderer renderer;
	BaseApp() {


	}
	virtual ~BaseApp() {
		glfwTerminate();
	}

	virtual void setup() {}
	virtual void update() {}
	virtual void draw() {}

	void initApp(const std::string& title, float width, float height) {
		window.init(title, width, height);
		shader.init();

	}

	void runApp() {

		renderer.drawRectangle();


		while(!glfwWindowShouldClose(window.handle)) {
			processInput(window.handle);


			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glUseProgram(shader.shaderID);
			glBindVertexArray(renderer.VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			glfwSwapBuffers(window.handle);


			glfwPollEvents();
		}
	}
};