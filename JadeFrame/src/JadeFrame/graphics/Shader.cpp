#include "Shader.h"

Shader::Shader()
	: m_ID()
	, m_shader_types() {

	if(1) {
		vertex_shader_source =
			R"(
			#version 450 core
			layout (location = 0) in vec3 v_Pos;
			layout (location = 1) in vec4 v_Col;

			out vec4 f_Col;

			uniform mat4 MVP;

			void main() {
				f_Col = v_Col;
				gl_Position = MVP * vec4(v_Pos.x, v_Pos.y, v_Pos.z, 1.0);
			}
		)";

		fragment_shader_source =
			R"(
			#version 450 core

			in vec4 f_Col;

			out vec4 o_Col;

			void main() {
				o_Col = f_Col;
			}
		)";
	} else {

		vertex_shader_source =
			R"(
				#version 450 core
				layout (location = 0) in vec3 v_Pos;
				layout (location = 1) in vec4 v_Col;

				uniform mat4 MVP;

				void main() {
					gl_Position = MVP * vec4(v_Pos.x, v_Pos.y, v_Pos.z, 1.0);
				}
			)";

		fragment_shader_source =
			R"(
				#version 450 core

				out vec4 o_Col;

				uniform vec4 color;

				void main() {
					o_Col = color;
				}
			)";
	}


}
void Shader::init() {

	this->compile(GL_VERTEX_SHADER, vertex_shader_source);
	this->compile(GL_FRAGMENT_SHADER, fragment_shader_source);
	this->link();
	this->validate();
	this->update_shader_variables(GL_ACTIVE_UNIFORMS);
	this->update_shader_variables(GL_ACTIVE_ATTRIBUTES);
	glDetachShader(m_ID, m_shader_types[0]);
	glDetachShader(m_ID, m_shader_types[1]);
}

void Shader::use() {
	glUseProgram(this->m_ID);
}

GLuint Shader::compile(GLenum type, const std::string& codeSource) {
	GLuint shader_ID = glCreateShader(type);
	const GLchar* shader_code = codeSource.c_str();
	glShaderSource(shader_ID, 1, &shader_code, nullptr);
	glCompileShader(shader_ID);

	switch(type) {
	case GL_VERTEX_SHADER:   m_shader_types[0] = shader_ID; break;
	case GL_FRAGMENT_SHADER: m_shader_types[1] = shader_ID; break;
	case GL_GEOMETRY_SHADER: m_shader_types[2] = shader_ID; break;
	default:; break;
	}

	GLint is_compiled = GL_FALSE;
	glGetShaderiv(shader_ID, GL_COMPILE_STATUS, &is_compiled);
	if(is_compiled == GL_FALSE) {
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

void Shader::link() {

	m_ID = glCreateProgram();
	glAttachShader(m_ID, m_shader_types[0]);
	glAttachShader(m_ID, m_shader_types[1]);


	glLinkProgram(m_ID);
	GLint is_linked = GL_FALSE;
	glGetProgramiv(m_ID, GL_LINK_STATUS, &is_linked);
	if(is_linked == GL_FALSE) {
		char info_log[1024];
		glGetProgramInfoLog(m_ID, 512, nullptr, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
	} else {
		std::cout << "SUCCE::SHADER::PROGRAM::LINKING_SUCCEEDED\n" << std::endl;
	}
}

void Shader::validate() {
	glValidateProgram(m_ID);

	GLint is_validated = GL_FALSE;
	glGetProgramiv(m_ID, GL_VALIDATE_STATUS, (int*)&is_validated);
	if(is_validated == GL_FALSE) {
		char info_log[1024];
		glGetProgramInfoLog(m_ID, 512, nullptr, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::VALIDATION_FAILED\n" << info_log << std::endl;
	} else {
		std::cout << "SUCCE::SHADER::PROGRAM::VALIDATION_SUCCEEDED\n" << std::endl;
	}
}


GLint Shader::get_uniform_location(const std::string& name) const {
	GLint location = glGetUniformLocation(m_ID, name.c_str());
	if(location == -1){
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


void Shader::set_uniform1i(const std::string& name, int value) {
	std::cout << __FUNCTION__ << " not implemented yet!" << std::endl; __debugbreak();
}
void Shader::set_uniform1f(const std::string& name, float value) {
	std::cout << __FUNCTION__ << " not implemented yet!" << std::endl; __debugbreak();
}
void Shader::set_uniform2f(const std::string& name, const Vec2& value) {
	std::cout << __FUNCTION__ << " not implemented yet!" << std::endl; __debugbreak();
}
void Shader::set_uniform3f(const std::string& name, const Vec3& value) {
	std::cout << __FUNCTION__ << " not implemented yet!" << std::endl; __debugbreak();
}
void Shader::set_uniform4f(const std::string& name, const Vec4& value) {
	int loc = get_uniform_location(name);
	glUniform4f(loc, value.x, value.y, value.z, value.w);
}
void Shader::set_uniform_matrix4fv(const std::string& name, const Mat4& mat) const {
	GLint loc = get_uniform_location(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

void Shader::update_shader_variables(int shaderType) {
	int num;
	glGetProgramiv(m_ID, shaderType, &num);

	// iterate over all active uniforms
	char buffer[128];
	if(shaderType == GL_ACTIVE_UNIFORMS) {
		m_uniforms.resize(num);
		for(unsigned int i = 0; i < num; ++i) {
			GLenum glType;
			glGetActiveUniform(m_ID, i, sizeof(buffer), 0, &m_uniforms[i].size, &glType, buffer);
			m_uniforms[i].name = std::string(buffer);
			m_uniforms[i].type = SHADER_TYPE::BOOL;
			m_uniforms[i].location = glGetUniformLocation(m_ID, buffer);
		}
	} else if(shaderType == GL_ACTIVE_ATTRIBUTES) {
		m_attributes.resize(num);
		for(unsigned int i = 0; i < num; ++i) {
			GLenum glType;
			glGetActiveAttrib(m_ID, i, sizeof(buffer), 0, &m_attributes[i].size, &glType, buffer);
			m_attributes[i].name = std::string(buffer);
			m_attributes[i].type = SHADER_TYPE::BOOL;
			m_attributes[i].location = glGetAttribLocation(m_ID, buffer);
		}
	} else {
		std::cout << __FUNCTION__ << " shaderType is false" << std::endl;
		__debugbreak();
	}
}