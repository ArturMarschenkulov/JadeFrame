#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "JadeFrame/math/Vec3.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

class Window {
public:
	Window() {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		handle = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
		if(handle == nullptr) {
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}
		glfwMakeContextCurrent(handle);
		glfwSetFramebufferSizeCallback(handle, framebuffer_size_callback);

		if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to initialize GLAD" << std::endl;
		}
	}
	GLFWwindow* handle;
};

class Shader {
public:
	Shader() {
		const char* vertexShaderSource = R"(
			#version 450 core
			layout (location = 0) in vec3 aPos;
			void main() {
			   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
			}
		)";


		const char* fragmentShaderSource = R"(
			#version 450 core
			out vec4 FragColor;
			void main() {
			   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
			}
		)";


		int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		// check for shader compile errors
		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// fragment shader
		int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		// check for shader compile errors
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// link shaders
		shaderID = glCreateProgram();
		glAttachShader(shaderID, vertexShader);
		glAttachShader(shaderID, fragmentShader);
		glLinkProgram(shaderID);
		// check for linking errors
		glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
		if(!success) {
			glGetProgramInfoLog(shaderID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
	GLint shaderID;
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

	void initApp() {
		renderer.drawRectangle();
	}

	void runApp() {
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