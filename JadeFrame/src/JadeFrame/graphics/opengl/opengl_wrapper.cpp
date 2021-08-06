#include "opengl_wrapper.h"
//#include <iostream>

namespace JadeFrame {


OGLW_VertexArray::OGLW_VertexArray() {
	glCreateVertexArrays(1, &m_ID);
}
OGLW_VertexArray::~OGLW_VertexArray() {
	this->reset();
}
OGLW_VertexArray::OGLW_VertexArray(OGLW_VertexArray&& other) noexcept : m_ID(other.release()) {
}
auto OGLW_VertexArray::bind() const -> void {
	assert(m_ID != 0);
	glBindVertexArray(m_ID);
}
auto OGLW_VertexArray::unbind() const -> void {
	glBindVertexArray(0);
}


//OGLW_Shader::OGLW_Shader(OGLW_Shader&& other) noexcept : m_ID(other.release()) {
//}
OGLW_Shader::OGLW_Shader(const GLenum type)
	: m_ID(glCreateShader(type)) {
}

OGLW_Shader::~OGLW_Shader() {
	this->reset();
}
auto OGLW_Shader::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}
auto OGLW_Shader::reset(GLuint ID) -> void {
	glDeleteShader(m_ID);
	m_ID = ID;
}
auto OGLW_Shader::set_source(const std::string& source_code) -> void {
	const GLchar* source = source_code.c_str();
	glShaderSource(m_ID, 1, &source, nullptr);
}
auto OGLW_Shader::compile() -> void {
	glCompileShader(m_ID);
}


#define GL_SHADER_BINARY_FORMAT_SPIR_V_ARB 0x9551
auto OGLW_Shader::set_binary(const std::vector<u32>& binary) -> void {
	glShaderBinary(
		1,
		&m_ID,
		GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
		binary.data(),
		binary.size()
	);
}
auto OGLW_Shader::compile_binary() -> void {
	__debugbreak();
	glSpecializeShader(m_ID, "main", 0, nullptr, nullptr);
	__debugbreak();	

}
auto OGLW_Shader::get_info(GLenum pname) -> GLint {
	GLint result;
	glGetShaderiv(m_ID, pname, &result);
	return result;
}
auto OGLW_Shader::get_compile_status() -> GLint {
	GLint is_compiled = GL_FALSE;
	glGetShaderiv(m_ID, GL_COMPILE_STATUS, &is_compiled);
	return is_compiled;
}
auto OGLW_Shader::get_info_log(GLsizei max_length) -> std::string {
	GLchar info_log[512];
	glGetShaderInfoLog(m_ID, max_length, &max_length, &info_log[0]);
	std::string result(info_log);
	return result;
}


OGLW_Program::OGLW_Program()
	: m_ID(glCreateProgram()) {
}
//OGLW_Program::OGLW_Program(OGLW_Program&& other) noexcept : m_ID(other.release()) {
//}
OGLW_Program::~OGLW_Program() {
	this->reset();
}
auto OGLW_Program::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}
auto OGLW_Program::reset(GLuint ID) -> void {
	glDeleteProgram(m_ID);
	m_ID = ID;
}
auto OGLW_Program::bind() const -> void {
	assert(m_ID != 0);
	glUseProgram(m_ID);
}
auto OGLW_Program::unbind() const -> void {
	glUseProgram(0);
}
auto OGLW_Program::attach(const OGLW_Shader& shader) const -> void {
	glAttachShader(m_ID, shader.m_ID);
}
auto OGLW_Program::link() const -> void {
	glLinkProgram(m_ID);

	GLint is_linked = GL_FALSE;
	glGetProgramiv(m_ID, GL_LINK_STATUS, &is_linked);
	if (is_linked == GL_FALSE) {
		char info_log[1024];
		glGetProgramInfoLog(m_ID, 512, nullptr, info_log);
		__debugbreak();
		//std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
	} else {
		//std::cout << "SUCCE::SHADER::PROGRAM::LINKING_SUCCEEDED\n" << std::endl;
	}
}
auto OGLW_Program::detach(const OGLW_Shader& shader) const -> void {
	glDetachShader(m_ID, shader.m_ID);
}
auto OGLW_Program::validate() const -> bool {
	glValidateProgram(m_ID);

	GLint is_validated = GL_FALSE;
	glGetProgramiv(m_ID, GL_VALIDATE_STATUS, (i32*)&is_validated);
	if (is_validated == GL_FALSE) {
		char info_log[1024];
		glGetProgramInfoLog(m_ID, 512, nullptr, info_log);
		__debugbreak();
		//std::cout << "ERROR::SHADER::PROGRAM::VALIDATION_FAILED\n" << info_log << std::endl;
		return false;
	} else {
		//std::cout << "SUCCE::SHADER::PROGRAM::VALIDATION_SUCCEEDED\n" << std::endl;
		return true;
	}
}
auto OGLW_Program::get_uniform_block_index(const char* name) const -> GLuint {
	return glGetUniformBlockIndex(m_ID, name);
}
auto OGLW_Program::set_uniform_block_binding(GLuint index, GLuint binding_point) const -> void {
	glUniformBlockBinding(m_ID, index, binding_point);
}
auto OGLW_Program::get_uniform_location(const std::string& name) const -> GLint {
	GLint location = glGetUniformLocation(m_ID, name.c_str());
	if (location == -1) {
		//std::cout << "Location of " << name << " can not be found" << std::endl;
		//__debugbreak();
	}
	return location;
}
auto OGLW_Program::get_attribute_location(const std::string& name) const -> GLint {
	GLint location = glGetAttribLocation(m_ID, name.c_str());
	if (location == -1) {
		//std::cout << "Location of " << name << " can not be found" << std::endl;
		__debugbreak();
	}
	return location;
}

auto OGLW_Program::get_info(GLenum pname) const -> GLint {
	// GL_DELETE_STATUS, GL_LINK_STATUS, GL_VALIDATE_STATUS, GL_INFO_LOG_LENGTH, GL_ATTACHED_SHADERS, GL_ACTIVE_ATTRIBUTES, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, GL_ACTIVE_UNIFORMS, GL_ACTIVE_UNIFORM_MAX_LENGTH.
	GLint result;
	glGetProgramiv(m_ID, pname, &result);
	return result;
}
auto OGLW_Program::get_info_log(GLsizei max_length) const -> std::string {
	GLchar info_log[512];
	glGetProgramInfoLog(m_ID, max_length, &max_length, &info_log[0]);
	std::string result(info_log);
	return result;
}


}	