#pragma once
#include <glad/glad.h>
#include "opengl_buffer.h" // TODO: FOr SHADER_DATA_TYPE. Maybe move?

#include <vector>
#include <string>
#include <iostream>

class Vec2;
class Vec3;
class Vec4;
class Mat4;
class OpenGL_Shader;

struct GLVariable {
	SHADER_DATA_TYPE  type;
	std::string  name;
	int          size;
	GLuint		 location;
};
struct GLShader {
	GLShader() = default;
	GLShader(const GLShader&) = delete;
	GLShader(GLShader&& other) : m_ID(other.release()) {
	}
	auto operator=(const GLShader&)->GLShader & = delete;
	auto operator=(GLShader&&)->GLShader & = delete;

	GLShader(const GLenum type)
		: m_ID(glCreateShader(type)) {
	}
	GLShader(const GLenum type, const std::string& source_code)
		: m_ID(glCreateShader(type)) {
		this->set_source(source_code);
		this->compile();


		GLint is_compiled = GL_FALSE;
		glGetShaderiv(m_ID, GL_COMPILE_STATUS, &is_compiled);

		if (is_compiled == GL_FALSE) {
			GLint max_length;
			glGetShaderiv(m_ID, GL_INFO_LOG_LENGTH, &max_length);
			GLchar info_log[512];
			glGetShaderInfoLog(m_ID, max_length, &max_length, &info_log[0]);
			glDeleteShader(m_ID);
			std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED" << info_log << std::endl;
		} else {
			//std::cout << "SUCCE::SHADER::PROGRAM::COMPILATION_SUCCEEDED" << std::endl;
		}

	}
	~GLShader() {
		this->reset();
	}
	auto release() -> GLuint {
		GLuint ret = m_ID;
		m_ID = 0;
		return ret;
	}
	auto reset(GLuint ID = 0) -> void {
		glDeleteShader(m_ID);
		m_ID = ID;
	}
	auto set_source(const std::string& source_code) -> void {
		const GLchar* source = source_code.c_str();
		glShaderSource(m_ID, 1, &source, nullptr);
	}
	auto query_source() const -> std::string {
		GLchar* source = nullptr;
		GLsizei buffer_size;
		glGetShaderSource(m_ID, 1000, &buffer_size, source);
		return std::string(source);
	}
	auto compile() -> void {
		glCompileShader(m_ID);
	}
	GLuint m_ID;
};
struct GLProgram {
	GLProgram()
		: m_ID(glCreateProgram()) {
	}
	GLProgram(const GLProgram&) = delete;
	GLProgram(GLProgram&& other) : m_ID(other.release()) {
	}
	auto operator=(const GLProgram&)->GLProgram & = delete;
	auto operator=(GLProgram&&)->GLProgram & = delete;
	~GLProgram() {
		this->reset();
	}
	auto release() -> GLuint {
		GLuint ret = m_ID;
		m_ID = 0;
		return ret;
	}
	auto reset(GLuint ID = 0) -> void {
		glDeleteProgram(m_ID);
		m_ID = ID;
	}
	auto bind() const -> void {
		glUseProgram(m_ID);
	}
	auto unbind() const -> void {
		glUseProgram(0);
	}
	auto attach(const GLShader& shader) const -> void {
		glAttachShader(m_ID, shader.m_ID);
	}
	auto link() const -> void {
		glLinkProgram(m_ID);

		GLint is_linked = GL_FALSE;
		glGetProgramiv(m_ID, GL_LINK_STATUS, &is_linked);
		if (is_linked == GL_FALSE) {
			char info_log[1024];
			glGetProgramInfoLog(m_ID, 512, nullptr, info_log);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
		} else {
			//std::cout << "SUCCE::SHADER::PROGRAM::LINKING_SUCCEEDED\n" << std::endl;
		}
	}
	auto detach(const GLShader& shader) const -> void {
		glDetachShader(m_ID, shader.m_ID);
	}
	auto validate() const -> bool {
		glValidateProgram(m_ID);

		GLint is_validated = GL_FALSE;
		glGetProgramiv(m_ID, GL_VALIDATE_STATUS, (int*)&is_validated);
		if (is_validated == GL_FALSE) {
			char info_log[1024];
			glGetProgramInfoLog(m_ID, 512, nullptr, info_log);
			std::cout << "ERROR::SHADER::PROGRAM::VALIDATION_FAILED\n" << info_log << std::endl;
			return false;
		} else {
			//std::cout << "SUCCE::SHADER::PROGRAM::VALIDATION_SUCCEEDED\n" << std::endl;
			return true;
		}
	}

	auto get_uniform_location(const std::string& name) const -> GLint {
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		if (location == -1) {
			std::cout << "Location of " << name << " can not be found" << std::endl;
			__debugbreak();
		}
		return location;
	}

	auto query_uniforms(const GLenum variable_type) const -> std::vector<GLVariable> {
		// variable_type = GL_ACTIVE_UNIFORMS | GL_ACTIVE_ATTRIBUTES

		GLint num_variables;
		glGetProgramiv(m_ID, variable_type, &num_variables);
		std::vector<GLVariable> variables(num_variables);
		for (int i = 0; i < num_variables; ++i) {
			char buffer[128];
			GLenum gl_type;
			switch (variable_type) {
				case GL_ACTIVE_UNIFORMS: glGetActiveUniform(m_ID, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer); break;
				case GL_ACTIVE_ATTRIBUTES: glGetActiveAttrib(m_ID, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer); break;
				default: __debugbreak(); gl_type = -1; break;
			}
			variables[i].name = std::string(buffer);
			variables[i].type = SHADER_TYPE_from_openGL_enum(gl_type);
			variables[i].location = glGetUniformLocation(m_ID, buffer);
		}
		return variables;
	}
	GLuint m_ID;
};

class OpenGL_Shader {
public:
	OpenGL_Shader() = default;
	OpenGL_Shader(const OpenGL_Shader&) = delete;
	OpenGL_Shader(OpenGL_Shader&& other)
		: m_program(std::move(other.m_program))
		, m_vertex_shader(std::move(other.m_vertex_shader))
		, m_fragment_shader(std::move(other.m_fragment_shader)) {

		m_vertex_source = other.m_vertex_source;
		m_fragment_source = other.m_fragment_source;
	}
	auto operator=(const OpenGL_Shader&)->OpenGL_Shader & = delete;
	auto operator=(OpenGL_Shader&&)->OpenGL_Shader & = delete;
	OpenGL_Shader(const std::string& name);
	auto bind() const -> void;
	auto unbind() const -> void;

//private:
	GLProgram m_program;
	GLShader m_vertex_shader;
	GLShader m_fragment_shader;
	std::string m_vertex_source;
	std::string m_fragment_source;


public:
	auto get_uniform_location(const std::string& name) const->GLint;
	auto set_uniform(const std::string& name, const int value) const -> void;
	auto set_uniform(const std::string& name, const unsigned int value) const -> void;
	auto set_uniform(const std::string& name, const float value) const -> void;
	auto set_uniform(const std::string& name, const Vec2& value) const -> void;
	auto set_uniform(const std::string& name, const Vec3& value) const -> void;
	auto set_uniform(const std::string& name, const Vec4& value) const -> void;
	auto set_uniform_matrix(const std::string& name, const Mat4& mat) const -> void;
	//std::array<std::vector<Variable>, 2> m_variables; //0 uniforms, 1 attributes
	std::vector<GLVariable> m_uniforms;
	std::vector<GLVariable> m_attributes;
};