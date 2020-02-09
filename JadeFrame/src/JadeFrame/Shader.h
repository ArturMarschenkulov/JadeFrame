#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

#include "math/Vec2.h"
#include "math/Vec3.h"
#include "math/Vec4.h"
#include "math/Mat4.h"

class Shader {
public:
	Shader() {}
	void init();
	GLuint compile(GLenum type, const std::string& codeSource);
	void link(GLuint vertexShader, GLuint fragmentShader);
	void validate();
	//private:
	GLuint shaderID = 0;

};
