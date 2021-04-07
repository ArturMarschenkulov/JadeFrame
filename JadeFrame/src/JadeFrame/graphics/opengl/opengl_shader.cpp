#include "opengl_shader.h"

#include "../../math/vec_2.h"
#include "../../math/vec_3.h"
#include "../../math/vec_4.h"
#include "../../math/mat_4.h"

#include "opengl_shader_loader.h"

#include<array>
#include<tuple>

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

OpenGL_Shader::OpenGL_Shader(const std::string& name)
	: m_program()
	, m_vertex_shader(GL_VERTEX_SHADER)
	, m_fragment_shader(GL_FRAGMENT_SHADER) {
		
	auto [vs_default, fs_default] = load_shader_by_name(name);


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
	//m_vertex_shader.query_source();
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


auto OpenGL_Shader::set_uniform(const std::string& name, const int value) -> void {

	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<int>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			auto& v = std::get<int>(m_uniforms[name].value);
			glUniform1i(m_uniforms[name].location, v);
			return;
		}
	}
	__debugbreak();
}
auto OpenGL_Shader::set_uniform(const std::string& name, const float value) -> void {
	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<float>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			auto& v = std::get<float>(m_uniforms[name].value);
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
			auto& v = std::get<Vec3>(m_uniforms[name].value);
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
			auto& v = std::get<Vec4>(m_uniforms[name].value);
			glUniform4f(m_uniforms[name].location, v.x, v.y, v.z, v.w);
			return;
		}
	}
	__debugbreak();
}
auto OpenGL_Shader::set_uniform(const std::string& name, const Mat4& value) -> void {
	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<Mat4>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			auto& v = std::get<Mat4>(m_uniforms[name].value);
			glUniformMatrix4fv(m_uniforms[name].location, 1, GL_FALSE, &v[0][0]);
			return;
		}
	}
	__debugbreak();
}

auto OpenGL_Shader::update_uniforms() -> void {
	for (auto& uniform : m_uniforms) {
		auto variable = uniform.second;
		if (std::holds_alternative<Mat4>(variable.value)) {
			glUniformMatrix4fv(variable.location, 1, GL_FALSE, &std::get<Mat4>(variable.value)[0][0]);
		} else if (std::holds_alternative<Vec3>(variable.value)) {
			auto& v = std::get<Vec3>(variable.value);
			glUniform3f(variable.location, v.x, v.y, v.z);
		} else {
			__debugbreak();
		}
	}
}

auto OpenGL_Shader::query_uniforms(const GLenum variable_type) const -> std::unordered_map<std::string, GLVariable> {
	// variable_type = GL_ACTIVE_UNIFORMS | GL_ACTIVE_ATTRIBUTES

	GLint num_variables = m_program.get_info(variable_type);
	std::vector<GLVariable> variables(num_variables);
	std::unordered_map<std::string, GLVariable> variable_map;

	for (int i = 0; i < num_variables; ++i) {
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
			GLValueVariant value_init;
			switch (variables[i].type) {
				case SHADER_TYPE::SAMPLER_2D:
					value_init = int(); break;
				case SHADER_TYPE::FLOAT:
					value_init = float(); break;
				case SHADER_TYPE::FLOAT_2:
					value_init = Vec2(); break;
				case SHADER_TYPE::FLOAT_3:
					value_init = Vec3(); break;
				case SHADER_TYPE::FLOAT_4:
					value_init = Vec4(); break;
				case SHADER_TYPE::MAT_4:
					value_init = Mat4(); break;
				default: __debugbreak(); break;
			}
			variables[i].value = value_init;
		}

		variable_map[variables[i].name] = variables[i];
	}
	return variable_map;
}