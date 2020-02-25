#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "math/Vec2.h"
class Window {
public:
	Window();
	void init(const std::string& title, float width, float height);
	GLFWwindow* get_handle();
	Vec2 get_size();
	float get_width();
	float get_height();

	void display_FPS() {
		static int FPS = 0;       // This will store our fps
		static float lastTime = 0.0f;       // This will hold the time from the last frame
		float currentTime = GetTickCount64() * 0.001f;
		++FPS;
		if(currentTime - lastTime > 1.0f) {
			lastTime = currentTime;
			if(1) {
				std::string title = m_title + " FPS: " + std::to_string(FPS);
				glfwSetWindowTitle(m_handle, title.c_str());
			}
			FPS = 0;
		}
	}
private:
	GLFWwindow* m_handle;
	Vec2 m_size;
	std::string m_title;

};