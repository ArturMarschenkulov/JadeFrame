#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
class Window {
public:
	Window();
	void init(const std::string& title, float width, float height);

	void displayFPS() {
		static int FPS = 0;       // This will store our fps
		static float lastTime = 0.0f;       // This will hold the time from the last frame
		float currentTime = GetTickCount64() * 0.001f;
		++FPS;
		if(currentTime - lastTime > 1.0f) {
			lastTime = currentTime;
			if(1) {
				std::string title = this->title + " FPS: " + std::to_string(FPS);
				glfwSetWindowTitle(handle, title.c_str());
			}
			FPS = 0;
		}
	}
//private:
	GLFWwindow* handle;
	float width;
	float height;
	std::string title;

};