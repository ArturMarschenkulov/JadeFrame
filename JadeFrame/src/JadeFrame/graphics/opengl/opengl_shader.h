#pragma once
#include <glad/glad.h>
#include "opengl_buffer.h" // TODO: FOr SHADER_DATA_TYPE. Maybe move?

#include "../../math/mat_4.h"
#include "../../math/vec_3.h"

#include <vector>
#include <string>
#include <iostream>

class Vec2;
class Vec3;
class Vec4;
class Mat4;
class OpenGL_Shader;

//struct GLVariable {
//	SHADER_DATA_TYPE  type;
//	std::string  name;
//	int          size;
//	GLuint		 location;
//};
#include <variant>
#include <unordered_map>
#include <functional>
#include <queue>
using GLValueVariant = std::variant<Vec3, Vec4, Mat4, int>;
struct GLVariable {
	SHADER_DATA_TYPE  type;
	std::string  name;
	int          size;
	GLuint		 location;
	GLValueVariant value;
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
	auto compile() -> void {
		glCompileShader(m_ID);
	}
	auto get_info(GLenum pname) -> GLint {
		GLint result;
		glGetShaderiv(m_ID, pname, &result);
		return result;
	}
	auto get_compile_status() -> GLint {
		GLint is_compiled = GL_FALSE;
		glGetShaderiv(m_ID, GL_COMPILE_STATUS, &is_compiled);
		return is_compiled;
	}
	auto get_info_log(GLsizei max_length) -> std::string {
		GLchar info_log[512];
		glGetShaderInfoLog(m_ID, max_length, &max_length, &info_log[0]);
		std::string result(info_log);
		return result;
	}
	GLuint m_ID;
};
struct GLProgram {
	GLProgram()
		: m_ID(glCreateProgram()) {
	}

	GLProgram(GLProgram&& other) : m_ID(other.release()) {
	}
	GLProgram(const GLProgram&) = delete;
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
	auto get_attribute_location(const std::string& name) const -> GLint {
		GLint location = glGetAttribLocation(m_ID, name.c_str());
		if (location == -1) {
			std::cout << "Location of " << name << " can not be found" << std::endl;
			__debugbreak();
		}
		return location;
	}
	auto get_info(GLenum pname) const -> GLint {
		// GL_DELETE_STATUS, GL_LINK_STATUS, GL_VALIDATE_STATUS, GL_INFO_LOG_LENGTH, GL_ATTACHED_SHADERS, GL_ACTIVE_ATTRIBUTES, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, GL_ACTIVE_UNIFORMS, GL_ACTIVE_UNIFORM_MAX_LENGTH.
		GLint result;
		glGetProgramiv(m_ID, pname, &result);
		return result;
	}

	auto get_info_log(GLsizei max_length) const -> std::string {
		GLchar info_log[512];
		glGetProgramInfoLog(m_ID, max_length, &max_length, &info_log[0]);
		std::string result(info_log);
		return result;
	}
	auto query_uniforms(const GLenum variable_type) const -> std::unordered_map<std::string, GLVariable> {
		// variable_type = GL_ACTIVE_UNIFORMS | GL_ACTIVE_ATTRIBUTES

		GLint num_variables = this->get_info(variable_type);
		std::vector<GLVariable> variables(num_variables);
		std::unordered_map<std::string, GLVariable> variable_map;

		for (int i = 0; i < num_variables; ++i) {
			char buffer[128];
			GLenum gl_type;
			switch (variable_type) {
				case GL_ACTIVE_UNIFORMS:
					glGetActiveUniform(m_ID, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer);
					variables[i].location = this->get_uniform_location(buffer);
					break;
				case GL_ACTIVE_ATTRIBUTES:
					glGetActiveAttrib(m_ID, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer);
					variables[i].location = this->get_attribute_location(buffer);
					break;
				default: __debugbreak(); gl_type = -1; break;
			}
			variables[i].name = std::string(buffer);
			variables[i].type = SHADER_TYPE_from_openGL_enum(gl_type);
			{ // TODO: It initializes the types for the variant type, for error checking. Consider whether this is neccessary.
				GLValueVariant value_init;
				switch (variables[i].type) {
					case SHADER_DATA_TYPE::FLOAT_3:
						value_init = Vec3(); break;
					case SHADER_DATA_TYPE::FLOAT_4:
						value_init = Vec4(); break;
					case SHADER_DATA_TYPE::MAT_4:
						value_init = Mat4(); break;
					default: __debugbreak(); break;
				}
				variables[i].value = value_init;
			}

			variable_map[variables[i].name] = variables[i];
		}
		return variable_map;
	}
	GLuint m_ID;
};

class OpenGL_Shader {
public:

	OpenGL_Shader(const std::string& name);

	OpenGL_Shader() = default;
	OpenGL_Shader(OpenGL_Shader&&) = default;

	OpenGL_Shader(const OpenGL_Shader&) = delete;
	auto operator=(const OpenGL_Shader&)->OpenGL_Shader & = delete;
	auto operator=(OpenGL_Shader&&)->OpenGL_Shader & = delete;

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

	auto set_uniform(const std::string& name, const Vec3& value) -> void;
	auto set_uniform(const std::string& name, const Mat4& mat) -> void;
	auto update_uniforms() -> void;
	std::unordered_map<std::string, GLVariable> m_uniforms;
	std::unordered_map<std::string, GLVariable> m_attributes;
};