#include "opengl_object.h"
#include <iostream>

namespace JadeFrame {


GL_VertexArray::GL_VertexArray() {
	glCreateVertexArrays(1, &m_ID);
}
GL_VertexArray::~GL_VertexArray() {
	this->reset();
}
GL_VertexArray::GL_VertexArray(GL_VertexArray&& other) noexcept : m_ID(other.release()) {
}
auto GL_VertexArray::bind() const -> void {
	glBindVertexArray(m_ID);
}
auto GL_VertexArray::unbind() const -> void {
	glBindVertexArray(0);
}


GL_Shader::GL_Shader(GL_Shader&& other) noexcept : m_ID(other.release()) {
}
GL_Shader::GL_Shader(const GLenum type)
	: m_ID(glCreateShader(type)) {
}
//GL_Shader::GL_Shader(const GLenum type, const std::string& source_code)
//	: m_ID(glCreateShader(type)) {
//	this->set_source(source_code);
//	this->compile();
//
//
//	GLint is_compiled = GL_FALSE;
//	glGetShaderiv(m_ID, GL_COMPILE_STATUS, &is_compiled);
//
//	if (is_compiled == GL_FALSE) {
//		GLint max_length;
//		glGetShaderiv(m_ID, GL_INFO_LOG_LENGTH, &max_length);
//		GLchar info_log[512];
//		glGetShaderInfoLog(m_ID, max_length, &max_length, &info_log[0]);
//		glDeleteShader(m_ID);
//		std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED" << info_log << std::endl;
//	} else {
//		//std::cout << "SUCCE::SHADER::PROGRAM::COMPILATION_SUCCEEDED" << std::endl;
//	}
//
//}
GL_Shader::~GL_Shader() {
	this->reset();
}
auto GL_Shader::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}
auto GL_Shader::reset(GLuint ID) -> void {
	glDeleteShader(m_ID);
	m_ID = ID;
}
auto GL_Shader::set_source(const std::string& source_code) -> void {
	const GLchar* source = source_code.c_str();
	glShaderSource(m_ID, 1, &source, nullptr);
}
auto GL_Shader::compile() -> void {
	glCompileShader(m_ID);
}


#define GL_SHADER_BINARY_FORMAT_SPIR_V_ARB 0x9551
auto GL_Shader::set_binary(const std::vector<u32>& binary) -> void {
	glShaderBinary(
		1,
		&m_ID,
		GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
		binary.data(),
		binary.size()
	);
}
auto GL_Shader::compile_binary() -> void {
	__debugbreak();
	glSpecializeShader(m_ID, "main", 0, nullptr, nullptr);
	__debugbreak();	

}
auto GL_Shader::get_info(GLenum pname) -> GLint {
	GLint result;
	glGetShaderiv(m_ID, pname, &result);
	return result;
}
auto GL_Shader::get_compile_status() -> GLint {
	GLint is_compiled = GL_FALSE;
	glGetShaderiv(m_ID, GL_COMPILE_STATUS, &is_compiled);
	return is_compiled;
}
auto GL_Shader::get_info_log(GLsizei max_length) -> std::string {
	GLchar info_log[512];
	glGetShaderInfoLog(m_ID, max_length, &max_length, &info_log[0]);
	std::string result(info_log);
	return result;
}


GL_Program::GL_Program()
	: m_ID(glCreateProgram()) {
}
GL_Program::GL_Program(GL_Program&& other) noexcept : m_ID(other.release()) {
}
GL_Program::~GL_Program() {
	this->reset();
}
auto GL_Program::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}
auto GL_Program::reset(GLuint ID) -> void {
	glDeleteProgram(m_ID);
	m_ID = ID;
}
auto GL_Program::bind() const -> void {
	glUseProgram(m_ID);
}
auto GL_Program::unbind() const -> void {
	glUseProgram(0);
}
auto GL_Program::attach(const GL_Shader& shader) const -> void {
	glAttachShader(m_ID, shader.m_ID);
}
auto GL_Program::link() const -> void {
	glLinkProgram(m_ID);

	GLint is_linked = GL_FALSE;
	glGetProgramiv(m_ID, GL_LINK_STATUS, &is_linked);
	if (is_linked == GL_FALSE) {
		char info_log[1024];
		glGetProgramInfoLog(m_ID, 512, nullptr, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
	} else {
		//std::cout << "SUCCE::SHADER::PROGRAM::LINKING_SUCCEEDED\n" << std::endl;
	}
}
auto GL_Program::detach(const GL_Shader& shader) const -> void {
	glDetachShader(m_ID, shader.m_ID);
}
auto GL_Program::validate() const -> bool {
	glValidateProgram(m_ID);

	GLint is_validated = GL_FALSE;
	glGetProgramiv(m_ID, GL_VALIDATE_STATUS, (i32*)&is_validated);
	if (is_validated == GL_FALSE) {
		char info_log[1024];
		glGetProgramInfoLog(m_ID, 512, nullptr, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::VALIDATION_FAILED\n" << info_log << std::endl;
		return false;
	} else {
		//std::cout << "SUCCE::SHADER::PROGRAM::VALIDATION_SUCCEEDED\n" << std::endl;
		return true;
	}
}
auto GL_Program::get_uniform_location(const std::string& name) const -> GLint {
	GLint location = glGetUniformLocation(m_ID, name.c_str());
	if (location == -1) {
		//std::cout << "Location of " << name << " can not be found" << std::endl;
		//__debugbreak();
	}
	return location;
}
auto GL_Program::get_attribute_location(const std::string& name) const -> GLint {
	GLint location = glGetAttribLocation(m_ID, name.c_str());
	if (location == -1) {
		std::cout << "Location of " << name << " can not be found" << std::endl;
		__debugbreak();
	}
	return location;
}
auto GL_Program::get_info(GLenum pname) const -> GLint {
	// GL_DELETE_STATUS, GL_LINK_STATUS, GL_VALIDATE_STATUS, GL_INFO_LOG_LENGTH, GL_ATTACHED_SHADERS, GL_ACTIVE_ATTRIBUTES, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, GL_ACTIVE_UNIFORMS, GL_ACTIVE_UNIFORM_MAX_LENGTH.
	GLint result;
	glGetProgramiv(m_ID, pname, &result);
	return result;
}
auto GL_Program::get_info_log(GLsizei max_length) const -> std::string {
	GLchar info_log[512];
	glGetProgramInfoLog(m_ID, max_length, &max_length, &info_log[0]);
	std::string result(info_log);
	return result;
}

//----------------------------

enum class BufferUsage : GLenum {
	/** Set once by the application and used infrequently for drawing. */
	STREAM_DRAW = GL_STREAM_DRAW,
	STREAM_READ = GL_STREAM_READ,
	STREAM_COPY = GL_STREAM_COPY,


	STATIC_DRAW = GL_STATIC_DRAW,
	STATIC_READ = GL_STATIC_READ,
	STATIC_COPY = GL_STATIC_COPY,

	DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
	DYNAMIC_READ = GL_DYNAMIC_READ,
	DYNAMIC_COPY = GL_DYNAMIC_COPY

};
enum class TargetHint : GLenum {
	ARRAY_BUFFER = GL_ARRAY_BUFFER,
	ATOMIC_COUNTER_BUFFER = GL_ATOMIC_COUNTER_BUFFER,
	COPY_READ_BUFFER = GL_COPY_READ_BUFFER,
	COPY_WRITE_BUFFER = GL_COPY_WRITE_BUFFER,
	DISPATCH_INDIRECT_BUFFER = GL_DISPATCH_INDIRECT_BUFFER,
	DRAW_INDIRECT_BUFFER = GL_DRAW_INDIRECT_BUFFER,
	ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
	PIXEL_PACK_BUFFER = GL_PIXEL_PACK_BUFFER,
	PIXEL_UNPACK_BUFFER = GL_PIXEL_UNPACK_BUFFER,
	SHADER_STORAGE_BUFFER = GL_SHADER_STORAGE_BUFFER,
	TEXTURE_BUFFER = GL_TEXTURE_BUFFER,
	TRANSFORM_FEEDBACK_BUFFER = GL_TRANSFORM_FEEDBACK_BUFFER,
	UNIFORM_BUFFER = GL_UNIFORM_BUFFER
};


}	