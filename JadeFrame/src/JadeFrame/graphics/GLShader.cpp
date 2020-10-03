#include "GLShader.h"

GLShader::GLShader()
	: m_ID() {
}

static auto compile(GLenum type, const std::string& code_source)->GLuint {
	GLuint shader_ID = glCreateShader(type);
	const GLchar* shader_code = code_source.c_str();
	glShaderSource(shader_ID, 1, &shader_code, nullptr);
	glCompileShader(shader_ID);

	GLint is_compiled = GL_FALSE;
	glGetShaderiv(shader_ID, GL_COMPILE_STATUS, &is_compiled);

	if (is_compiled == GL_FALSE) {
		GLint max_length = 512;
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



const GLchar* vs_source_flat =
R"(
			#version 450 core
			layout (location = 0) in vec3 v_pos;
			layout (location = 1) in vec4 v_col;
			layout (location = 2) in vec2 v_texture_coord;

			out vec4 f_col;
			out vec2 f_texture_coord;

			uniform mat4 MVP;

			void main() {
				gl_Position = MVP * vec4(v_pos.x, v_pos.y, v_pos.z, 1.0);
				f_col = v_col;
				f_texture_coord = v_texture_coord;
			}
		)";
const GLchar* fs_source_flat =
R"(
			#version 450 core

			in vec4 f_col;
			in vec2 f_texture_coord;
			out vec4 o_col;

			uniform sampler2D texture_0;

			void main() {
				
				o_col = texture(texture_0, f_texture_coord);
				o_col = f_col;
			}
		)";

const GLchar* fs_source_flat_2 =
R"(
			#version 450 core

			in vec4 f_col;

			out vec4 o_col;


			float near = 0.1;
			float far = 10.0;

			void main() {

				float z_ndc = gl_FragCoord.z * 2.0 - 1.0;
				float z_clip = z_ndc / gl_FragCoord.w;
				float depth2 = (z_clip + near) / (near + far);

				float depth3 = (2.0 * near * far) / (far + near - z_ndc * (near - far));



				o_Col = vec4(vec3(depth2), 1.0);
			}
		)";


auto GLShader::init() -> void {
	//const GLchar* vertex_shader_source = vs_source_flat;
	//const GLchar* fragment_shader_source = fs_source_flat;

	GLuint vertex_shader_id = compile(GL_VERTEX_SHADER, vs_source_flat);
	GLuint fragment_shader_id = compile(GL_FRAGMENT_SHADER, fs_source_flat);

	GLuint program_id = link(vertex_shader_id, fragment_shader_id);
	validate(program_id);

	glDetachShader(program_id, vertex_shader_id);
	glDetachShader(program_id, fragment_shader_id);

	m_ID = program_id;

	m_variables[0] = update_shader_variables(GL_ACTIVE_UNIFORMS);
	m_variables[1] = update_shader_variables(GL_ACTIVE_ATTRIBUTES);
}

auto GLShader::use() -> void {
	glUseProgram(this->m_ID);
}

auto GLShader::get_uniform_location(const std::string& name) const -> GLint {
	GLint location = glGetUniformLocation(
		m_ID, 
		name.c_str());
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


auto GLShader::set_uniform(const std::string& name, int value) const -> void {
	int loc = get_uniform_location(name);
	glUniform1i(loc, value);
}
auto GLShader::set_uniform(const std::string& name, float value) const -> void {
	int loc = get_uniform_location(name);
	glUniform1f(loc, value);
}
auto GLShader::set_uniform(const std::string& name, const Vec2& value) const -> void {
	int loc = get_uniform_location(name);
	glUniform2f(loc, value.x, value.y);
}
auto GLShader::set_uniform(const std::string& name, const Vec3& value) const -> void {
	int loc = get_uniform_location(name);
	glUniform3f(loc, value.x, value.y, value.z);
}
auto GLShader::set_uniform(const std::string& name, const Vec4& value) const -> void {
	int loc = get_uniform_location(name);
	glUniform4f(loc, value.x, value.y, value.z, value.w);
}
auto GLShader::set_uniform_matrix(const std::string& name, const Mat4& mat) const -> void {
	GLint loc = get_uniform_location(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

auto GLShader::update_shader_variables(GLenum variable_type) -> std::vector<GLShader::Variable> {
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