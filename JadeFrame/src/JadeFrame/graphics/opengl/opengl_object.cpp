#include "opengl_object.h"
#include <iostream>

GLVertexBuffer::GLVertexBuffer() {
	glCreateBuffers(1, &m_ID);
}
GLVertexBuffer::~GLVertexBuffer() {
	this->reset();
}
GLVertexBuffer::GLVertexBuffer(GLVertexBuffer&& other) : m_ID(other.release()) {
}
auto GLVertexBuffer::bind() const -> void {
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}
auto GLVertexBuffer::unbind() const -> void {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
auto GLVertexBuffer::reserve(GLuint size_in_bytes) const -> void {
	// if NULL is passed in as data, it only reserves size_in_bytes bytes.
	glBufferData(GL_ARRAY_BUFFER, size_in_bytes, NULL, GL_STATIC_DRAW);
	//glNamedBufferData(m_ID, size_in_bytes, NULL, buffer_usage);
}
auto GLVertexBuffer::send(const std::vector<float>& data) const -> void {
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
	//glNamedBufferData(m_ID, size_in_bytes, data, buffer_usage);
}
auto GLVertexBuffer::update(GLuint size_in_bytes, const void* data) const -> void {
	glBufferSubData(GL_ARRAY_BUFFER, 0, size_in_bytes, data);
	//glNamedBufferSubData(m_ID, 0, size_in_bytes, data);
}

GLVertexArray::GLVertexArray() {
	glCreateVertexArrays(1, &m_ID);
}
GLVertexArray::~GLVertexArray() {
	this->release();
}
GLVertexArray::GLVertexArray(GLVertexArray&& other) : m_ID(other.release()) {
}
auto GLVertexArray::bind() const -> void {
	glBindVertexArray(m_ID);
}
auto GLVertexArray::unbind() const -> void {
	glBindVertexArray(0);
}

GLIndexBuffer::GLIndexBuffer() {
	glCreateBuffers(1, &m_ID);
}
GLIndexBuffer::~GLIndexBuffer() {
	this->release();
}
GLIndexBuffer::GLIndexBuffer(GLIndexBuffer&& other) : m_ID(other.release()) {
}
auto GLIndexBuffer::bind() const -> void {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
}
auto GLIndexBuffer::unbind() const -> void {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
auto GLIndexBuffer::reserve(GLuint size_in_bytes) const -> void {
	// if NULL is passed in as data, it only reserves size_in_bytes bytes.
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, NULL, GL_STATIC_DRAW);
	//glNamedBufferData(m_ID, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
}
auto GLIndexBuffer::send(const std::vector<GLuint>& indices) const -> void {
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	//glNamedBufferData(m_ID, size_in_bytes, data, GL_DYNAMIC_DRAW);
}
auto GLIndexBuffer::update(GLuint size_in_bytes, const GLuint* data) const -> void {
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size_in_bytes, data);
	//glNamedBufferSubData(m_ID, 0, size_in_bytes, data);
}



GLShader::GLShader(GLShader&& other) : m_ID(other.release()) {
}
GLShader::GLShader(const GLenum type)
	: m_ID(glCreateShader(type)) {
}
GLShader::GLShader(const GLenum type, const std::string& source_code)
	: m_ID(glCreateShader(type)) {
	this->set_source(source_code);
	this->compile();


	GLint is_compiled = GL_FALSE;
	glGetShaderiv(m_ID, GL_COMPILE_STATUS, &is_compiled);

	if (is_compiled == GL_FALSE) {
		GLint max_length;
		glGetShaderiv(m_ID, GL_INFO_LOG_LENGTH, &max_length);
		GLchar info_log[512];
		glGetShaderInfoLog(m_ID, max_length, &max_length, &info_log[0]);
		glDeleteShader(m_ID);
		std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED" << info_log << std::endl;
	} else {
		//std::cout << "SUCCE::SHADER::PROGRAM::COMPILATION_SUCCEEDED" << std::endl;
	}

}
GLShader::~GLShader() {
	this->reset();
}
auto GLShader::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}
auto GLShader::reset(GLuint ID) -> void {
	glDeleteShader(m_ID);
	m_ID = ID;
}
auto GLShader::set_source(const std::string& source_code) -> void {
	const GLchar* source = source_code.c_str();
	glShaderSource(m_ID, 1, &source, nullptr);
}
auto GLShader::compile() -> void {
	glCompileShader(m_ID);
}
auto GLShader::get_info(GLenum pname) -> GLint {
	GLint result;
	glGetShaderiv(m_ID, pname, &result);
	return result;
}
auto GLShader::get_compile_status() -> GLint {
	GLint is_compiled = GL_FALSE;
	glGetShaderiv(m_ID, GL_COMPILE_STATUS, &is_compiled);
	return is_compiled;
}
auto GLShader::get_info_log(GLsizei max_length) -> std::string {
	GLchar info_log[512];
	glGetShaderInfoLog(m_ID, max_length, &max_length, &info_log[0]);
	std::string result(info_log);
	return result;
}


GLProgram::GLProgram()
	: m_ID(glCreateProgram()) {
}
GLProgram::GLProgram(GLProgram&& other) : m_ID(other.release()) {
}
GLProgram::~GLProgram() {
	this->reset();
}
auto GLProgram::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}
auto GLProgram::reset(GLuint ID) -> void {
	glDeleteProgram(m_ID);
	m_ID = ID;
}
auto GLProgram::bind() const -> void {
	glUseProgram(m_ID);
}
auto GLProgram::unbind() const -> void {
	glUseProgram(0);
}
auto GLProgram::attach(const GLShader& shader) const -> void {
	glAttachShader(m_ID, shader.m_ID);
}
auto GLProgram::link() const -> void {
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
auto GLProgram::detach(const GLShader& shader) const -> void {
	glDetachShader(m_ID, shader.m_ID);
}
auto GLProgram::validate() const -> bool {
	glValidateProgram(m_ID);

	GLint is_validated = GL_FALSE;
	glGetProgramiv(m_ID, GL_VALIDATE_STATUS, (int*)&is_validated);
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
auto GLProgram::get_uniform_location(const std::string& name) const -> GLint {
	GLint location = glGetUniformLocation(m_ID, name.c_str());
	if (location == -1) {
		std::cout << "Location of " << name << " can not be found" << std::endl;
		__debugbreak();
	}
	return location;
}
auto GLProgram::get_attribute_location(const std::string& name) const -> GLint {
	GLint location = glGetAttribLocation(m_ID, name.c_str());
	if (location == -1) {
		std::cout << "Location of " << name << " can not be found" << std::endl;
		__debugbreak();
	}
	return location;
}
auto GLProgram::get_info(GLenum pname) const -> GLint {
	// GL_DELETE_STATUS, GL_LINK_STATUS, GL_VALIDATE_STATUS, GL_INFO_LOG_LENGTH, GL_ATTACHED_SHADERS, GL_ACTIVE_ATTRIBUTES, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, GL_ACTIVE_UNIFORMS, GL_ACTIVE_UNIFORM_MAX_LENGTH.
	GLint result;
	glGetProgramiv(m_ID, pname, &result);
	return result;
}
auto GLProgram::get_info_log(GLsizei max_length) const -> std::string {
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

struct GLBuffer {

};