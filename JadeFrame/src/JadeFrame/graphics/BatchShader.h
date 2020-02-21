#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "../math/Vec2.h"
#include "../math/Vec3.h"
#include "../math/Vec4.h"
#include "../math/Mat4.h"


class BatchShader {
public:
	BatchShader();
	void init();
	void use();
	GLuint compile(GLenum type, const std::string& codeSource);
	void link();
	void validate();
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
		GLuint location;
	};
	struct Uniform : ShaderType {};
	struct VertexAttribute : ShaderType {};
public:
	GLint get_uniform_location(const std::string& name) const;
	//void setMat4(const std::string& name, const Mat4& mat) const;
	void set_uniform1i(const std::string& name, const int value);
	void set_uniform1f(const std::string& name, const float value);
	void set_uniform2f(const std::string& name, const Vec2& value);
	void set_uniform3f(const std::string& name, const Vec3& value);
	void set_uniform4f(const std::string& name, const Vec4& value);
	void set_uniform_matrix4fv(const std::string& name, const Mat4& mat) const;
	void update_shader_variables(int shaderType);
	std::vector<Uniform> m_uniforms;
	std::vector<VertexAttribute> m_attributes;
};
