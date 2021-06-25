#include "opengl_shader.h"

#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/vec_3.h"
#include "JadeFrame/math/vec_4.h"
#include "JadeFrame/math/mat_4.h"

#include "opengl_shader_loader.h"

#include<array>
#include<tuple>
#include <JadeFrame/graphics/glsl_parser.h>
#include <cassert>

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
OpenGL_Shader::OpenGL_Shader(const std::string& name)
	: m_program()
	, m_vertex_shader(GL_VERTEX_SHADER)
	, m_fragment_shader(GL_FRAGMENT_SHADER) {

	auto [vs_default, fs_default] = get_shader_by_name(name);

	GLSLParser parser;
	parser.parse(vs_default);
	assert(parser.m_variables.size() == 8);
	//__debugbreak();
	parser.parse(fs_default);
	//__debugbreak();
	//exit(0);


	m_vertex_shader.set_source(vs_default);
	m_vertex_shader.compile();

	m_fragment_shader.set_source(fs_default);
	m_fragment_shader.compile();

	m_program.attach(m_vertex_shader);
	m_program.attach(m_fragment_shader);
	m_program.link();
	m_program.validate();

	m_program.detach(m_vertex_shader);
	m_program.detach(m_fragment_shader);


	m_vertex_source = vs_default;
	m_fragment_source = fs_default;
	m_uniforms = this->query_uniforms(GL_ACTIVE_UNIFORMS);
	m_attributes = this->query_uniforms(GL_ACTIVE_ATTRIBUTES);

	{
		// Check whether m_uniforms and m_attributes are all contained in parser.
		for (auto uniform = m_uniforms.begin(); uniform != m_uniforms.end(); uniform++) {
			bool exists = false;
			for (int j = 0; j < parser.m_variables.size(); j++) {
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
			for (int j = 0; j < parser.m_variables.size(); j++) {
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
				glGetActiveUniform(m_program.m_ID, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer);
				variables[i].location = m_program.get_uniform_location(buffer);
				break;
			case GL_ACTIVE_ATTRIBUTES:
				glGetActiveAttrib(m_program.m_ID, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer);
				variables[i].location = m_program.get_attribute_location(buffer);
				break;
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