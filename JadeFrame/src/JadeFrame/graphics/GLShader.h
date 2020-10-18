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
//private:
	GLuint m_ID = 0;
//private:
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

	struct Variable {
		SHADER_TYPE  type;
		std::string  name;
		int          size;
		GLuint		 location;
	};


public:
	auto get_uniform_location(const std::string& name) const -> GLint;
	auto set_uniform(const std::string& name, const int value) const -> void;
	auto set_uniform(const std::string& name, const float value) const -> void;
	auto set_uniform(const std::string& name, const Vec2& value) const -> void;
	auto set_uniform(const std::string& name, const Vec3& value) const -> void;
	auto set_uniform(const std::string& name, const Vec4& value) const -> void;
	auto set_uniform_matrix(const std::string& name, const Mat4& mat) const -> void;

	auto update_shader_variables(GLenum variable_type) -> std::vector<GLShader::Variable>;
	std::array<std::vector<Variable>, 2> m_variables; //0 uniforms, 1 attributes
};

//auto print_GPU_variable_info(GLShader* shader) -> void {
//	std::cout << "Uniforms: ";
//	for (auto uniform : shader->m_variables[0]) {
//		std::cout << uniform.name << " ";
//	} std::cout << std::endl;
//
//	std::cout << "Attribs : ";
//	for (auto attributes : shader->m_variables[1]) {
//		std::cout << attributes.name << " ";
//	} std::cout << std::endl;
//}