#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
class Shader {
public:
	Shader() {}
	void init();
	GLuint compile(GLenum type, const std::string& codeSource);
	void link(GLuint vertexShader, GLuint fragmentShader);
	GLuint shaderID = 0;
};




class Renderer {
public:

	GLuint VBO = 0;
	GLuint VAO = 0;
	GLuint IBO = 0;
	Renderer() {}

	void drawRectangle() {
		GLfloat vertices[] = {
			 0.5f,  0.5f, 0.0f,  // top right
			 0.5f, -0.5f, 0.0f,  // bottom right
			-0.5f, -0.5f, 0.0f,  // bottom left
			-0.5f,  0.5f, 0.0f   // top left 
		};
		GLuint indices[] = {  // note that we start from 0!
			0, 1, 3,  // first Triangle
			1, 2, 3   // second Triangle
		};


		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0); glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	}
};