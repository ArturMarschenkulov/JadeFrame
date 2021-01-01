#include "GLShader.h"
#include<array>
#include<tuple>



static auto compile(GLenum type, const std::string& code_source)->GLuint {
	GLuint shader_ID = glCreateShader(type);
	const GLchar* shader_code = code_source.c_str();
	glShaderSource(shader_ID, 1, &shader_code, nullptr);
	glCompileShader(shader_ID);

	GLint is_compiled = GL_FALSE;
	glGetShaderiv(shader_ID, GL_COMPILE_STATUS, &is_compiled);

	if (is_compiled == GL_FALSE) {
		GLint max_length;
		glGetShaderiv(shader_ID, GL_INFO_LOG_LENGTH, &max_length);
		GLchar info_log[512];
		glGetShaderInfoLog(shader_ID, max_length, &max_length, &info_log[0]);
		glDeleteShader(shader_ID);
		std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED" << info_log << std::endl;
		return 0;
	} else {
		std::cout << "SUCCE::SHADER::PROGRAM::COMPILATION_SUCCEEDED" << std::endl;
	}
	return shader_ID;
}
static auto link(GLuint vertex_shader_id, GLuint fragment_shader_id)->GLuint {
	GLuint program_ID = glCreateProgram();

	glAttachShader(program_ID, vertex_shader_id);
	glAttachShader(program_ID, fragment_shader_id);

	glLinkProgram(program_ID);
	GLint is_linked = GL_FALSE;
	glGetProgramiv(program_ID, GL_LINK_STATUS, &is_linked);
	if (is_linked == GL_FALSE) {
		char info_log[1024];
		glGetProgramInfoLog(program_ID, 512, nullptr, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
	} else {
		std::cout << "SUCCE::SHADER::PROGRAM::LINKING_SUCCEEDED\n" << std::endl;
	}
	return program_ID;
}
static auto validate(GLuint program_id) -> void {
	glValidateProgram(program_id);

	GLint is_validated = GL_FALSE;
	glGetProgramiv(program_id, GL_VALIDATE_STATUS, (int*)&is_validated);
	if (is_validated == GL_FALSE) {
		char info_log[1024];
		glGetProgramInfoLog(program_id, 512, nullptr, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::VALIDATION_FAILED\n" << info_log << std::endl;
	} else {
		std::cout << "SUCCE::SHADER::PROGRAM::VALIDATION_SUCCEEDED\n" << std::endl;
	}
}
static auto get_default_shader_0() -> std::tuple<std::string, std::string> {
	static const GLchar* vs_default =
		R"(
			#version 450 core
			layout (location = 0) in vec3 v_position;
			layout (location = 1) in vec4 v_color;
			layout (location = 2) in vec2 v_texture_coord;

			out vec4 f_color;
			out vec2 f_texture_coord;

			uniform mat4 MVP;
			uniform mat4 view_projection;
			uniform mat4 projection;
			uniform mat4 view;
			uniform mat4 model;

			void main() {
				gl_Position = view_projection * model * vec4(v_position, 1.0);
				f_color = v_color;
				f_texture_coord = v_texture_coord;
			}
	)";
	static const GLchar* fs_default =
		R"(
			#version 450 core

			in vec4 f_color;
			in vec2 f_texture_coord;
			out vec4 o_color;

			uniform sampler2D texture_0;

			void main() {
				
				o_color = mix(f_color, texture(texture_0, f_texture_coord), 0.8);
				o_color = texture(texture_0, f_texture_coord);
				o_color = f_color;
			}
	)";

	return std::make_tuple(std::string(vs_default), std::string(fs_default));
}
static auto get_default_shader_1() -> std::tuple<std::string, std::string> {
	static const GLchar* vs_default =
		R"(
			#version 450 core
			layout (location = 0) in vec3 v_position;
			layout (location = 1) in vec4 v_color;
			layout (location = 2) in vec2 v_texture_coord;

			out vec4 f_color;
			out vec2 f_texture_coord;

			uniform mat4 MVP;
			uniform mat4 view_projection;
			uniform mat4 projection;
			uniform mat4 view;
			uniform mat4 model;

			void main() {
				gl_Position = view_projection * model * vec4(v_position, 1.0);
				f_color = v_color;
				f_texture_coord = v_texture_coord;
			}
	)";
	static const GLchar* fs_default =
		R"(
			#version 450 core

			in vec4 f_color;
			in vec2 f_texture_coord;
			out vec4 o_color;

			uniform sampler2D texture_0;

			void main() {
				
				o_color = mix(f_color, texture(texture_0, f_texture_coord), 0.8);
				o_color = texture(texture_0, f_texture_coord);
				//o_color = f_color;
			}
	)";

	return std::make_tuple(std::string(vs_default), std::string(fs_default));
}

static auto get_shader_variables_0(GLuint shader_id, GLenum variable_type) -> std::vector<GLShader::Variable> {
	GLint variable_amount;
	glGetProgramiv(shader_id, variable_type, &variable_amount);
	std::vector<GLShader::Variable> variables;
	variables.resize(variable_amount);

	char buffer[128];
	for (int i = 0; i < variable_amount; ++i) {
		GLenum gl_type;
		switch (variable_type) {
		case GL_ACTIVE_UNIFORMS:
		{
			glGetActiveUniform(shader_id, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer);
		} break;
		case GL_ACTIVE_ATTRIBUTES:
		{
			glGetActiveAttrib(shader_id, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer);
		}break;
		default: __debugbreak(); break;
		}
		variables[i].name = std::string(buffer);
		variables[i].type = GLShader::SHADER_TYPE::BOOL;
		variables[i].location = glGetUniformLocation(shader_id, buffer);
	}
	return variables;
}

GLShader::GLShader(int num)
	: m_ID() {

	//auto [vs_default, fs_default] = get_default_shader_0();
	std::string vs_default, fs_default;
	switch (num) {
	case 0: std::tie(vs_default, fs_default) = get_default_shader_0(); break;
	case 1: std::tie(vs_default, fs_default) = get_default_shader_1(); break;
	default: break;
	}

	GLuint vertex_shader_id = compile(GL_VERTEX_SHADER, vs_default);
	GLuint fragment_shader_id = compile(GL_FRAGMENT_SHADER, fs_default);
	m_ID = link(vertex_shader_id, fragment_shader_id);
	validate(m_ID);

	glDetachShader(m_ID, vertex_shader_id);
	glDetachShader(m_ID, fragment_shader_id);

	m_uniforms = get_shader_variables_0(m_ID, GL_ACTIVE_UNIFORMS);
	m_attributes = get_shader_variables_0(m_ID, GL_ACTIVE_ATTRIBUTES);
}

auto GLShader::init(int num) -> GLShader {
	return GLShader(num);
}

auto GLShader::bind() -> void {
	glUseProgram(m_ID);
}
auto GLShader::unbind() -> void {
	glUseProgram(0);
}

auto GLShader::get_uniform_location(const std::string& name) const -> GLint {
	GLint location = glGetUniformLocation(m_ID, name.c_str());
	if (location == -1) {
		std::cout << "Location of " << name << " can not be found" << std::endl;
		__debugbreak();
	}
	return location;
	//if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end()) {
	//	return m_uniformLocationCache[name];
	//} else {
	//	GLint location = glGetUniformLocation(m_ID, name.c_str());
	//	m_uniformLocationCache[name] = location;
	//	return location;
	//}

		// read from uniform/attribute array as originally obtained from OpenGL
	//for(unsigned int i = 0; i < m_uniforms.size(); ++i) {
	//	if(m_uniforms[i].name == name)
	//		return m_uniforms[i].location;
	//}
	//std::cout << "Location of " << name << " can not be found" << std::endl;
	//__debugbreak();
	//return -1;
}


auto GLShader::set_uniform(const std::string& name, const int value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform1i(loc, value);
}

auto GLShader::set_uniform(const std::string& name, const unsigned int value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform1ui(loc, value);
}
auto GLShader::set_uniform(const std::string& name, float value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform1f(loc, value);
}
auto GLShader::set_uniform(const std::string& name, const Vec2& value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform2f(loc, value.x, value.y);
}
auto GLShader::set_uniform(const std::string& name, const Vec3& value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform3f(loc, value.x, value.y, value.z);
}
auto GLShader::set_uniform(const std::string& name, const Vec4& value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform4f(loc, value.x, value.y, value.z, value.w);
}
auto GLShader::set_uniform_matrix(const std::string& name, const Mat4& mat) const -> void {
	GLint loc = get_uniform_location(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

auto GLShader::get_shader_variables(GLenum variable_type) -> std::vector<GLShader::Variable> {
	GLint num;
	glGetProgramiv(m_ID, variable_type, &num);

	// iterate over all active uniforms
	int variable_type_id = -1;
	switch (variable_type) {
	case GL_ACTIVE_UNIFORMS: variable_type_id = 0; break;
	case GL_ACTIVE_ATTRIBUTES: variable_type_id = 1; break;
	default:
	{
		variable_type_id = -1;
		std::cout << __FUNCTION__ << " shader_type is false" << std::endl;
		__debugbreak();
	} break;
	}

	char buffer[128];
	std::vector<GLShader::Variable> m_variables;
	m_variables.resize(num);
	for (int i = 0; i < num; ++i) {
		GLenum gl_type;
		switch (variable_type_id) {
		case 0: glGetActiveUniform(m_ID, i, sizeof(buffer), 0, &m_variables[i].size, &gl_type, buffer); break;
		case 1: glGetActiveAttrib(m_ID, i, sizeof(buffer), 0, &m_variables[i].size, &gl_type, buffer); break;
		}
		m_variables[i].name = std::string(buffer);
		m_variables[i].type = GLShader::SHADER_TYPE::BOOL;
		m_variables[i].location = glGetUniformLocation(m_ID, buffer);
	}
	return m_variables;
}





//-------------------------------


struct Shader {
	Shader(GLenum shader_type) {
		m_ID = glCreateShader(shader_type);
	}
	~Shader() {
		glDeleteShader(m_ID);
	}
	auto set_shader_source(const std::string& code_source) -> void {
		const GLchar* shader_code = code_source.c_str();
		glShaderSource(m_ID, 1, &shader_code, nullptr);
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

struct Program {
	Program() {
		m_ID = glCreateProgram();
	}
	auto init() -> void {
		//m_shaders.push_back({ GL_VERTEX_SHADER });
		//m_shaders[0].set_shader_source(vs_source_flat);
		//m_shaders[0].compile();

		//m_shaders.push_back({ GL_FRAGMENT_SHADER });
		//m_shaders[1].set_shader_source(vs_source_flat);
		//m_shaders[1].compile();

		//Program program;
		//program.attach(v_shader);
		//program.attach(f_shader);
		//program.link();


		//program.detach(v_shader);
		//program.detach(f_shader);
	}
	auto attach(const Shader& shader) -> void {
		glAttachShader(m_ID, shader.m_ID);
	}
	auto detach(const Shader& shader) -> void {
		glDetachShader(m_ID, shader.m_ID);
	}
	auto link() -> void {
		glLinkProgram(m_ID);
	}
	auto get_info(GLenum pname) -> GLint {
		GLint result;
		glGetProgramiv(m_ID, pname, &result);
		return result;
	}
	auto use() -> void {
		glUseProgram(m_ID);
	}

	auto get_info_log(GLsizei max_length) -> std::string {
		GLchar info_log[512];
		glGetProgramInfoLog(m_ID, max_length, &max_length, &info_log[0]);
		std::string result(info_log);
		return result;
	}
	GLuint m_ID;
	std::vector<Shader> m_shaders; //0:vertex_shader, 1: fragment_shader
};

struct GLShaderProgram {
	Program program;
};