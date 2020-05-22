#pragma once
#include <glad/glad.h>
#include <vector>

#include "../math/Vec2.h"
#include "../math/Vec3.h"
#include "../math/Vec4.h"
#include "../math/Mat4.h"


class GLShader {
public:
	GLShader();
	auto init() -> void;
	auto use() -> void;
	auto compile(GLenum type, const std::string& codeSource) -> GLuint;
	auto link() -> void;
	auto validate() -> void;
private:
	GLuint m_ID = 0;
	std::array<GLuint, 3> m_shader_types; // Vertex, Fragment, Geometrys

	const GLchar* vertex_shader_source;
	const GLchar* fragment_shader_source;
private:
	enum class SHADER_TYPE {
		BOOL,
		INT,
		FLOAT,
		SAMPLER1D,
		SAMPLER2D,
		SAMPLER3D,
		SAMPLERCUBE,
		VEC2,
		VEC3,
		VEC4,
		MAT2,
		MAT3,
		MAT4
	};

	struct ShaderType {
		SHADER_TYPE  type;
		std::string  name;
		int          size;
		GLuint		 location;
	};
	struct Uniform : ShaderType {};
	struct VertexAttribute : ShaderType {};
public:
	auto get_uniform_location(const std::string& name) const -> GLint;
	auto set_uniform(const std::string& name, const int value) const -> void;
	auto set_uniform(const std::string& name, const float value) const -> void;
	auto set_uniform(const std::string& name, const Vec2& value) const -> void;
	auto set_uniform(const std::string& name, const Vec3& value) const -> void;
	auto set_uniform(const std::string& name, const Vec4& value) const -> void;
	auto set_uniform_matrix(const std::string& name, const Mat4& mat) const -> void;
	auto update_shader_variables(int shaderType) -> void;
	std::vector<Uniform> m_uniforms;
	std::vector<VertexAttribute> m_attributes;
};
