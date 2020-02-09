#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Window {
public:
	Window();
	void init(const std::string& title, float width, float height);
//private:
	GLFWwindow* handle;
};