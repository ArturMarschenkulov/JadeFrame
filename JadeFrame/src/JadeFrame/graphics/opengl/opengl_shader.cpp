#include "opengl_shader.h"

#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/vec_3.h"
#include "JadeFrame/math/vec_4.h"
#include "JadeFrame/math/mat_4.h"

#include "opengl_shader_loader.h"

#include "../to_spirv.h"
#include <future>

#include<array>
#include<tuple>
#include <JadeFrame/graphics/glsl_parser.h>
#include <cassert>

namespace JadeFrame {
static auto SHADER_TYPE_from_openGL_enum(const GLenum type) -> SHADER_TYPE {
	switch (type) {
		case GL_FLOAT: return SHADER_TYPE::FLOAT;  break;
		case GL_FLOAT_VEC2: return SHADER_TYPE::FLOAT_2;  break;
		case GL_FLOAT_VEC3: return SHADER_TYPE::FLOAT_3; break;
		case GL_FLOAT_VEC4: return SHADER_TYPE::FLOAT_4; break;
		case GL_FLOAT_MAT4: return SHADER_TYPE::MAT_4; break;
		case GL_SAMPLER_2D: return SHADER_TYPE::SAMPLER_2D; break;
		default: __debugbreak(); return {};
	}
}
//static auto check_glsl_variables(const std::unordered_map<std::string, OpenGL_Shader::GL_Variable>& ) -> void {
//
//}

OpenGL_Shader::OpenGL_Shader(const GLSLCode& code) 
	: m_program()
	, m_vertex_shader(GL_VERTEX_SHADER)
	, m_fragment_shader(GL_FRAGMENT_SHADER) {

	if constexpr (false) {
		//auto [vertex_shader_code, fragment_shader_code] = get_shader_by_name("spirv_test_0");
		std::future<std::vector<u32>> vert_shader_spirv = std::async(std::launch::async, string_to_SPIRV, code.m_vertex_shader.c_str(), 0);
		std::future<std::vector<u32>> frag_shader_spirv = std::async(std::launch::async, string_to_SPIRV, code.m_fragment_shader.data(), 1);

		std::vector<u32> mvert_shader_spirv = vert_shader_spirv.get();
		std::vector<u32> mfrag_shader_spirv = frag_shader_spirv.get();

		m_fragment_shader.set_binary(mfrag_shader_spirv);
		m_fragment_shader.compile_binary();

		m_vertex_shader.set_binary(mvert_shader_spirv);
		m_vertex_shader.compile_binary();


	} else {
		m_vertex_shader.set_source(code.m_vertex_shader);
		m_vertex_shader.compile();

		m_fragment_shader.set_source(code.m_fragment_shader);
		m_fragment_shader.compile();
	}

	m_program.attach(m_vertex_shader);
	m_program.attach(m_fragment_shader);
	m_program.link();
	m_program.validate();

	m_program.detach(m_vertex_shader);
	m_program.detach(m_fragment_shader);




	m_vertex_source = code.m_vertex_shader;
	m_fragment_source = code.m_fragment_shader;
	m_uniforms = this->query_uniforms(GL_ACTIVE_UNIFORMS);
	m_attributes = this->query_uniforms(GL_ACTIVE_ATTRIBUTES);



	if constexpr (false) {

		GLSLParser parser;
		parser.parse(code.m_vertex_shader);
		parser.parse(code.m_fragment_shader);
		// Check whether m_uniforms and m_attributes are all contained in parser.
		for (auto uniform = m_uniforms.begin(); uniform != m_uniforms.end(); uniform++) {
			bool exists = false;
			for (u32 j = 0; j < parser.m_variables.size(); j++) {
				const GLSLParser::Variable& parser_variable = parser.m_variables[j];
				const std::string& uniform_name = uniform->first;
				if (uniform_name == parser_variable.m_name) {
					exists = true;
				}
			}
			if (exists == false) __debugbreak();
		}
		for (auto attribute = m_attributes.begin(); attribute != m_attributes.end(); attribute++) {
			bool exists = false;
			for (u32 j = 0; j < parser.m_variables.size(); j++) {
				const GLSLParser::Variable& parser_variable = parser.m_variables[j];
				const std::string& uniform_name = attribute->first;
				if (uniform_name == parser_variable.m_name) {
					exists = true;
				}
			}
			if (exists == false) __debugbreak();
		}
	}
}

auto OpenGL_Shader::bind() const -> void {
	m_program.bind();
}
auto OpenGL_Shader::unbind() const -> void {
	m_program.unbind();
}

auto OpenGL_Shader::get_uniform_location(const std::string& name) const -> GLint {
	if (m_uniforms.contains(name)) {
		return m_uniforms.at(name).location;
	}
	__debugbreak();
	return -1;
}


auto OpenGL_Shader::set_uniform(const std::string& name, const i32 value) -> void {

	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<i32>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			i32& v = std::get<i32>(m_uniforms[name].value);
			glUniform1i(m_uniforms[name].location, v);
			return;
		}
	}
	__debugbreak();
}
auto OpenGL_Shader::set_uniform(const std::string& name, const f32 value) -> void {
	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<f32>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			f32& v = std::get<f32>(m_uniforms[name].value);
			glUniform1f(m_uniforms[name].location, v);
			return;
		}
	}
	__debugbreak();
}
auto OpenGL_Shader::set_uniform(const std::string& name, const Vec3& value) -> void {
	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<Vec3>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			Vec3& v = std::get<Vec3>(m_uniforms[name].value);
			glUniform3f(m_uniforms[name].location, v.x, v.y, v.z);
			return;
		}
	}
	__debugbreak();
}
auto OpenGL_Shader::set_uniform(const std::string& name, const Vec4& value) -> void {
	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<Vec4>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			Vec4& v = std::get<Vec4>(m_uniforms[name].value);
			glUniform4f(m_uniforms[name].location, v.x, v.y, v.z, v.w);
			return;
		}
	}
	__debugbreak();
}
auto OpenGL_Shader::set_uniform(const std::string& name, const Matrix4x4& value) -> void {
	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<Matrix4x4>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			Matrix4x4& v = std::get<Matrix4x4>(m_uniforms[name].value);
			glUniformMatrix4fv(m_uniforms[name].location, 1, GL_FALSE, &v[0][0]);
			return;
		}
	}
	__debugbreak();
}

auto OpenGL_Shader::set_uniform_block(const std::string& name, const std::vector<Matrix4x4>& mat) -> void {
	m_uniform_buffer.bind();
	m_uniform_buffer.send(mat);
	m_uniform_buffer.unbind();
}

auto OpenGL_Shader::update_uniforms() -> void {
	for (auto& uniform : m_uniforms) {
		GL_Variable variable = uniform.second;
		if (std::holds_alternative<Matrix4x4>(variable.value)) {
			Matrix4x4& v = std::get<Matrix4x4>(variable.value);
			glUniformMatrix4fv(variable.location, 1, GL_FALSE, &v[0][0]);
		} else if (std::holds_alternative<Vec3>(variable.value)) {
			Vec3& v = std::get<Vec3>(variable.value);
			glUniform3f(variable.location, v.x, v.y, v.z);
		} else {
			__debugbreak();
		}
	}
}
#include "opengl_object.h"

auto OpenGL_Shader::query_uniforms(const GLenum variable_type) const -> std::unordered_map<std::string, GL_Variable> {
	// variable_type = GL_ACTIVE_UNIFORMS | GL_ACTIVE_ATTRIBUTES

	GLint num_variables = m_program.get_info(variable_type);
	std::vector<GL_Variable> variables(num_variables);
	std::unordered_map<std::string, GL_Variable> variable_map;

	for (i32 i = 0; i < num_variables; ++i) {
		char buffer[128];
		GLenum gl_type;
		switch (variable_type) {
			case GL_ACTIVE_UNIFORMS:
			{
				glGetActiveUniform(m_program.m_ID, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer);
				GLint location = m_program.get_uniform_location(buffer);
				if (location == -1) {

					m_uniform_buffer.bind();
					m_uniform_buffer.reserve(16 + 16);
					m_uniform_buffer.unbind();

					const u32 binding_point = 0;
					//GLint index = glGetUniformBlockIndex(m_program.m_ID, buffer);
					//glUniformBlockBinding(m_program.m_ID, index, binding_point);

					glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_uniform_buffer.m_ID);
				} else {
					variables[i].location = location;
				}
			}
			break;
			case GL_ACTIVE_ATTRIBUTES:
			{
				glGetActiveAttrib(m_program.m_ID, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer);
				variables[i].location = m_program.get_attribute_location(buffer);
			}break;
			default: __debugbreak(); gl_type = -1; break;
		}
		variables[i].name = std::string(buffer);
		variables[i].type = SHADER_TYPE_from_openGL_enum(gl_type);

		{ // TODO: It initializes the types for the variant type, for error checking. Consider whether this is neccessary.
			GL_ValueVariant value_init;
			switch (variables[i].type) {
				case SHADER_TYPE::SAMPLER_2D: value_init = i32(); break;
				case SHADER_TYPE::FLOAT: value_init = f32(); break;
				case SHADER_TYPE::FLOAT_2: value_init = Vec2(); break;
				case SHADER_TYPE::FLOAT_3: value_init = Vec3(); break;
				case SHADER_TYPE::FLOAT_4: value_init = Vec4(); break;
				case SHADER_TYPE::MAT_4: value_init = Matrix4x4(); break;
				default: __debugbreak(); break;
			}
			variables[i].value = value_init;
		}

		variable_map[variables[i].name] = variables[i];
	}
	return variable_map;
}
}