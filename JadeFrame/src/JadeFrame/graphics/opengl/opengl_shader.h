#pragma once
#include <glad/glad.h>
#include "opengl_buffer.h" // TODO: FOr SHADER_DATA_TYPE. Maybe move?

#include "../../math/mat_4.h"
#include "../../math/vec_2.h"
#include "../../math/vec_3.h"

#include <vector>
#include <string>
#include <iostream>
#include <variant>
#include <unordered_map>
#include <functional>
#include <queue>

class Vec2;
class Vec3;
class Vec4;
class Matrix4x4;
class OpenGL_Shader;


using GLValueVariant = 
std::variant<
	int, float,
	Vec2, Vec3, Vec4,
	Matrix4x4
>;
struct GLVariable {
	SHADER_TYPE  type;
	std::string  name;
	int          size;
	GLuint		 location;
	GLValueVariant value;
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

	auto set_uniform(const std::string& name, const int value) -> void;
	auto set_uniform(const std::string& name, const float value) -> void;
	auto set_uniform(const std::string& name, const Vec3& value) -> void;
	auto set_uniform(const std::string& name, const Vec4& value) -> void;
	auto set_uniform(const std::string& name, const Matrix4x4& mat) -> void;
	auto update_uniforms() -> void;
	auto query_uniforms(const GLenum variable_type) const -> std::unordered_map<std::string, GLVariable>;

	std::unordered_map<std::string, GLVariable> m_uniforms;
	std::unordered_map<std::string, GLVariable> m_attributes;
};