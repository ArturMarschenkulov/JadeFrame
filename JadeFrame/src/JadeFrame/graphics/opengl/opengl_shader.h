#pragma once
#include <glad/glad.h>
#include "opengl_buffer.h" // TODO: FOr SHADER_DATA_TYPE. Maybe move?
#include "../shader_loader.h"

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/math/vec.h"

#include <vector>
#include <string>
#include <variant>
#include <unordered_map>
#include <functional>
#include <queue>

namespace JadeFrame {
class Matrix4x4;
class OpenGL_Shader;

class OpenGL_Shader : public IShader {
public:
	OpenGL_Shader(const DESC& desc);

	OpenGL_Shader() = default;
	OpenGL_Shader(OpenGL_Shader&&) noexcept = delete;

	OpenGL_Shader(const OpenGL_Shader&) = delete;
	auto operator=(const OpenGL_Shader&)->OpenGL_Shader & = delete;
	auto operator=(OpenGL_Shader&&)->OpenGL_Shader & = delete;

	auto bind() const -> void;
	auto unbind() const -> void;

private:
	OGLW_Program m_program;
	OGLW_Shader m_vertex_shader;
	OGLW_Shader m_fragment_shader;
	std::string m_vertex_source;
	std::string m_fragment_source;


public:
	auto get_uniform_location(const std::string& name) const->GLint;

private:
	using GL_ValueVariant =
		std::variant<
		i32, f32,
		v2, v3, v4,
		Matrix4x4
		>;
	struct GL_Variable {
		SHADER_TYPE  type;
		std::string  name;
		i32          size;
		GLuint		 location;
		GL_ValueVariant value;
	};
	auto query_uniforms(const GLenum variable_type) const->std::unordered_map<std::string, GL_Variable>;
private:

	std::unordered_map<std::string, GL_Variable> m_uniforms;
	std::unordered_map<std::string, GL_Variable> m_attributes;
};
}