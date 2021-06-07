#include "opengl_shader.h"

#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/vec_3.h"
#include "JadeFrame/math/vec_4.h"
#include "JadeFrame/math/mat_4.h"

#include "opengl_shader_loader.h"

#include<array>
#include<tuple>
namespace Parser {
/*
	This should be a parser for GLSL.
	The main purpose right now is to simply be able to get what attributes and uniforms a shader has.

*/
static auto is_digit(const char c)				-> bool { return c >= '0' && c <= '9'; }
static auto is_binary_digit(const char c)		-> bool { return c >= '0' && c <= '1'; }
static auto is_octal_digit(const char c)		-> bool { return c >= '0' && c <= '7'; }
static auto is_hexadecimal_digit(const char c)	-> bool { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
static auto is_alpha(const char c)				-> bool { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
static auto is_alpha_numeric(const char c)		-> bool { return is_alpha(c) || is_digit(c); }
class CharCursor {
public:
	CharCursor() {
	}
	CharCursor(const std::string& str) {
		it = &str[0];
	}
	auto operator=(std::string& str) -> CharCursor& {
		it = &str[0];
		return *this;
	}
	auto operator=(const std::string& str) -> CharCursor& {
		it = &str[0];
		return *this;
	}
	auto get() const -> const char*;
	auto advance() const -> void;
	auto peek(int n) const -> char;
private:
	mutable const char* it;
};
auto CharCursor::get() const -> const char* {
	return it;
}
auto CharCursor::advance() const -> void {
	it++;
}
auto CharCursor::peek(int n) const -> char {
	return *(it + n);
}


struct GLSLParser {
	struct VersionNumber {
		i32 m_number;
		enum class PROFILE {
			CORE,
			COMPATIBILITY,
		} m_profile;
	} m_version;
	struct Variable {
		enum class QUALIFIER {
			IN,
			OUT,
			UNIFORM,
		} m_qualifier;
		enum class TYPE {
			BOOL,
			INT, UINT, 
			FLOAT, DOUBLE,

			BVEC1, BVEC2, BVEC3, BVEC4,
			IVEC1, IVEC2, IVEC3, IVEC4,
			UVEC1, UVEC2, UVEC3, UVEC4,
			VEC1, VEC2, VEC3, VEC4,
			DVEC1, DVEC2, DVEC3, DVEC4,

			MAT4
		} m_type;
		i32 m_layout = -1; //NOTE: -1 means there is no layout. Consider a different approach.
		std::string m_name;
	};
	std::vector<Variable> m_variables;
	auto parse_name(const CharCursor& current_cursor, Variable& v) -> void {
		CharCursor prev_cursor = current_cursor;
		while (is_alpha_numeric(current_cursor.peek(1))) {
			current_cursor.advance();
		}
		std::string name(prev_cursor.get(), current_cursor.get() + 1);

		v.m_name = name;
	}
	auto parse_type(const CharCursor& current_cursor, Variable& v) -> void {
		CharCursor prev_cursor = current_cursor;
		while (is_alpha_numeric(current_cursor.peek(1))) {
			current_cursor.advance();
		}
		std::string type(prev_cursor.get(), current_cursor.get() + 1);
		if (type == "vec2") {
			v.m_type = Variable::TYPE::VEC2;
		} else if (type == "vec3") {
			v.m_type = Variable::TYPE::VEC3;
		} else if (type == "vec4") {
			v.m_type = Variable::TYPE::VEC4;
		} else if (type == "mat4") {
			v.m_type = Variable::TYPE::MAT4;
		} else {
			__debugbreak();
		}
	}
	auto parse_layout(const CharCursor& current_cursor, Variable& v) -> void {
		CharCursor prev_cursor = current_cursor;

		if (current_cursor.peek(0) != '(') __debugbreak();

		while (current_cursor.peek(0) != ')') {
			if (is_digit(current_cursor.peek(0))) {
				prev_cursor = current_cursor;
				while (is_digit(current_cursor.peek(1))) {
					current_cursor.advance();
				}

				std::string layout_number(prev_cursor.get(), current_cursor.get() + 1);
				v.m_layout = std::atoi(layout_number.c_str());
			}
			current_cursor.advance();
		}
	}
	auto parse_qualifier(const CharCursor& current_cursor, Variable& v) -> void {
		CharCursor prev_cursor = current_cursor;

		while (is_alpha_numeric(current_cursor.peek(1))) {
			current_cursor.advance();
		}
		std::string qualifier(prev_cursor.get(), current_cursor.get() + 1);
		if (qualifier == "in") {
			v.m_qualifier = Variable::QUALIFIER::IN;
		} else if (qualifier == "out") {
			v.m_qualifier = Variable::QUALIFIER::OUT;
		} else if (qualifier == "uniform") {
			v.m_qualifier = Variable::QUALIFIER::UNIFORM;
		} else {
			__debugbreak();
		}
	}
	auto parse(const std::string& text) -> void {
		/*
		in variable
		out variable
		uniform variable

		*/
		CharCursor start_cursor = text;
		CharCursor current_cursor = text;
		while (current_cursor.peek(0)) {
			char c = current_cursor.peek(0);
			switch (c) {
				case '\n': break;
				default: {
					start_cursor = current_cursor;
					CharCursor prev_cursor = current_cursor;

					while (is_alpha_numeric(current_cursor.peek(1))) {
						current_cursor.advance();
					}

					std::string word(prev_cursor.get(), current_cursor.get() + 1);
					
					if(word == "#version") {
						current_cursor.advance();
						current_cursor.advance(); // to skip whitespace
						prev_cursor = current_cursor;

						while (is_alpha_numeric(current_cursor.peek(1))) {
							current_cursor.advance();
						}
						std::string version_number(prev_cursor.get(), current_cursor.get() + 1);
						m_version.m_number = std::atoi(version_number.c_str());

						current_cursor.advance();
						current_cursor.advance(); // to skip whitespace
						prev_cursor = current_cursor;

						if(current_cursor.peek(0) == 'c') {
							while (is_alpha_numeric(current_cursor.peek(1))) {
								current_cursor.advance();
							}
							std::string profile(prev_cursor.get(), current_cursor.get() + 1);
							if(profile == "core") {
								m_version.m_profile = VersionNumber::PROFILE::CORE;
							} else if (profile == "compatibility") {
								m_version.m_profile = VersionNumber::PROFILE::COMPATIBILITY;
							} else {
								__debugbreak();
							}
						}

					} else if (word == "layout") {
						// parse layout number
						current_cursor.advance();
						current_cursor.advance(); // to skip whitespace

						Variable v;
						this->parse_layout(current_cursor, v);

						// Parse Qualifier
						current_cursor.advance();
						current_cursor.advance(); // to skip whitespace
						this->parse_qualifier(current_cursor, v);


						// Parse type
						current_cursor.advance();
						current_cursor.advance(); // to skip whitespace
						this->parse_type(current_cursor, v);
	

						// Parse name
						current_cursor.advance();
						current_cursor.advance(); // to skip whitespace
						this->parse_name(current_cursor, v);

						current_cursor.advance();
						if (current_cursor.peek(0) == ';') {
							
						} else {
							__debugbreak();
						}

						m_variables.push_back(v);
						//__debugbreak();
					} else if(word == "out" || word == "in" || word == "uniform") {
						Variable v;

						// Parse Qualifier
						//current_cursor.advance();
						//current_cursor.advance(); // to skip whitespace
						//this->parse_qualifier(current_cursor, v);
						std::string qualifier = word;
						if (qualifier == "in") {
							v.m_qualifier = Variable::QUALIFIER::IN;
						} else if (qualifier == "out") {
							v.m_qualifier = Variable::QUALIFIER::OUT;
						} else if (qualifier == "uniform") {
							v.m_qualifier = Variable::QUALIFIER::UNIFORM;
						} else {
							__debugbreak();
						}

						// Parse type
						current_cursor.advance();
						current_cursor.advance(); // to skip whitespace
						this->parse_type(current_cursor, v);


						// Parse name
						current_cursor.advance();
						current_cursor.advance(); // to skip whitespace
						this->parse_name(current_cursor, v);

						current_cursor.advance();
						if (current_cursor.peek(0) == ';') {

						} else {
							__debugbreak();
						}

						m_variables.push_back(v);
						//__debugbreak();

					}	
				}
			}
			current_cursor.advance();


		}
		__debugbreak();
	}

};
}

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

	auto [vs_default, fs_default] = get_shader_by_name(name);

	Parser::GLSLParser parser;
	parser.parse(vs_default);


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


auto OpenGL_Shader::set_uniform(const std::string& name, const i32 value) -> void {

	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<i32>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			auto& v = std::get<i32>(m_uniforms[name].value);
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
			auto& v = std::get<f32>(m_uniforms[name].value);
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
auto OpenGL_Shader::set_uniform(const std::string& name, const Matrix4x4& value) -> void {
	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<Matrix4x4>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			auto& v = std::get<Matrix4x4>(m_uniforms[name].value);
			glUniformMatrix4fv(m_uniforms[name].location, 1, GL_FALSE, &v[0][0]);
			return;
		}
	}
	__debugbreak();
}

auto OpenGL_Shader::update_uniforms() -> void {
	for (auto& uniform : m_uniforms) {
		auto variable = uniform.second;
		if (std::holds_alternative<Matrix4x4>(variable.value)) {
			glUniformMatrix4fv(variable.location, 1, GL_FALSE, &std::get<Matrix4x4>(variable.value)[0][0]);
		} else if (std::holds_alternative<Vec3>(variable.value)) {
			auto& v = std::get<Vec3>(variable.value);
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