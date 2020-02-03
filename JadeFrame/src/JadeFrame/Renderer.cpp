#include "Renderer.h"

void Shader::init() {
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
	GLuint vertexShader   = this->compile(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = this->compile(GL_FRAGMENT_SHADER, fragmentShaderSource);
	this->link(vertexShader, fragmentShader);
}

GLuint Shader::compile(GLenum type, const std::string& codeSource) {
	GLuint shaderID = glCreateShader(type);
	const GLchar* shaderCode = codeSource.c_str();
	glShaderSource(shaderID, 1, &shaderCode, nullptr);
	glCompileShader(shaderID);

	GLint isCompiled = GL_FALSE;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		GLint maxLength = 512;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);
		GLchar infoLog[512];
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, &infoLog[0]);
		glDeleteShader(shaderID);
		return 0;
	} else {

	}

	return shaderID;
}

void Shader::link(GLuint vertexShader, GLuint fragmentShader) {

	// link shaders
	shaderID = glCreateProgram();
	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);
	glLinkProgram(shaderID);
	// check for linking errors
	GLint isLinekd = GL_FALSE;
	glGetProgramiv(shaderID, GL_LINK_STATUS, &isLinekd);
	if(!isLinekd) {
		char infoLog[1024];
		glGetProgramInfoLog(shaderID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}
