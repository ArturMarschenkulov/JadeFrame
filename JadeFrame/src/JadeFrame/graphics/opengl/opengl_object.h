#pragma once
/*
	This is the place for the light OpenGL Wrapper classes.
	The goal is to make OpenGL objects more C++ like.
	This may or may not be temporary.

	These wrapper classes should have as little state as possible. The optimal would be to only have their ID and nothing more
*/
#include "JadeFrame/defines.h"
#include <glad/glad.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <cassert>

namespace JadeFrame {

/*******************
*	BUFFER
*******************/
template<GLenum buffer_type>
class GL_Buffer {
public:

	GL_Buffer(const GL_Buffer<buffer_type>&) = delete;
	auto operator=(const GL_Buffer<buffer_type>&)->GL_Buffer<buffer_type> & = delete;
	auto operator=(GL_Buffer<buffer_type>&&)->GL_Buffer<buffer_type> & = delete;

	GL_Buffer();
	GL_Buffer(GL_Buffer<buffer_type>&& other) noexcept;
	~GL_Buffer();

	auto bind() const -> void;
	auto unbind() const -> void;
	auto reserve(GLuint size_in_bytes) const -> void;
	template<typename U>
	auto send(const std::vector<U>& data) const -> void;
	auto update(GLuint size_in_bytes, const void* data) const -> void;

	auto bind_base(GLuint index) const -> void;
	auto bind_buffer_range(GLuint index, GLintptr offset, GLsizeiptr size) const -> void;

//private:
	GLuint m_ID;

private:
	auto release()->GLuint;
	auto reset(GLuint ID = 0) -> void;
};

template<GLenum buffer_type>
auto GL_Buffer<buffer_type>::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}
template<GLenum buffer_type>
auto GL_Buffer<buffer_type>::reset(GLuint ID) -> void {
	glDeleteBuffers(1, &m_ID);
	m_ID = ID;
}

template<GLenum buffer_type>
GL_Buffer<buffer_type>::GL_Buffer() {
	//glGenBuffers(1, &m_ID);
	glCreateBuffers(1, &m_ID);
	glBindBuffer(buffer_type, m_ID);
}
template<GLenum buffer_type>
GL_Buffer<buffer_type>::GL_Buffer(GL_Buffer<buffer_type>&& other) noexcept 
	: m_ID(other.release()) {
}
template<GLenum buffer_type>
GL_Buffer<buffer_type>::~GL_Buffer() {
	this->reset();
}
template<GLenum buffer_type>
auto GL_Buffer<buffer_type>::bind() const -> void {
	//glBindBuffer(buffer_type, m_ID);
}
template<GLenum buffer_type>
auto GL_Buffer<buffer_type>::unbind() const -> void {
	//glBindBuffer(buffer_type, 0);
}
template<GLenum buffer_type>
auto GL_Buffer<buffer_type>::reserve(GLuint size_in_bytes) const -> void {
	// if NULL is passed in as data, it only reserves size_in_bytes bytes.
	//glBufferData(buffer_type, size_in_bytes, NULL, GL_STATIC_DRAW);
	glNamedBufferData(m_ID, size_in_bytes, NULL, GL_STATIC_DRAW);
}

template<GLenum buffer_type>
template<typename U>
auto GL_Buffer<buffer_type>::send(const std::vector<U>& data) const -> void {
	//glBufferData(buffer_type, data.size() * sizeof(U), data.data(), GL_STATIC_DRAW);
	glNamedBufferData(m_ID, data.size() * sizeof(U), data.data(), GL_STATIC_DRAW);
}
template<GLenum buffer_type>
auto GL_Buffer<buffer_type>::update(GLuint size_in_bytes, const void* data) const -> void {
	//glBufferSubData(buffer_type, 0, size_in_bytes, data);
	glNamedBufferSubData(m_ID, 0, size_in_bytes, data);
}
template<GLenum buffer_type>
auto GL_Buffer<buffer_type>::bind_base(GLuint binding_point) const -> void {
	assert(GL_UNIFORM_BUFFER == buffer_type);
	glBindBufferBase(buffer_type, binding_point, m_ID);
}
template<GLenum buffer_type>
auto GL_Buffer<buffer_type>::bind_buffer_range(GLuint index, GLintptr offset, GLsizeiptr size) const -> void {
	assert(GL_UNIFORM_BUFFER == buffer_type);
	glBindBufferRange(buffer_type, index, m_ID, offset, size);
}

/*******************
*	VERTEX ARRAY
*******************/
class GL_VertexArray {
public:
	GL_VertexArray();
	~GL_VertexArray();
	GL_VertexArray(GL_VertexArray&& other) noexcept;

	GL_VertexArray(GL_VertexArray&) = delete;
	auto operator=(const GL_VertexArray&)->GL_VertexArray & = delete;
	auto operator=(GL_VertexArray&&)->GL_VertexArray & = delete;



	auto bind() const -> void;
	auto unbind() const -> void;
private:
	GLuint m_ID;
private:
	auto release() -> GLuint {
		GLuint ret = m_ID;
		m_ID = 0;
		return ret;
	}
	auto reset(GLuint ID = 0) -> void {
		glDeleteVertexArrays(1, &m_ID);
		m_ID = ID;
	}
};

/*******************
*	SHADER
*******************/

struct GL_Shader {
	GL_Shader() = default;
	GL_Shader(GL_Shader&& other) noexcept = delete;
	GL_Shader(const GLenum type);
	//GL_Shader(const GLenum type, const std::string& source_code);
	~GL_Shader();

	GL_Shader(const GL_Shader&) = delete;
	auto operator=(const GL_Shader&)->GL_Shader & = delete;
	auto operator=(GL_Shader&&)->GL_Shader & = delete;


	auto set_source(const std::string& source_code) -> void;
	auto compile() -> void;
	auto set_binary(const std::vector<u32>& binary) -> void;
	auto compile_binary() -> void;
	auto get_info(GLenum pname)->GLint;
	auto get_compile_status()->GLint;
	auto get_info_log(GLsizei max_length)->std::string;
public:
	GLuint m_ID;
private:

	auto release()->GLuint;
	auto reset(GLuint ID = 0) -> void;
};
/*******************
*	PROGRAM
*******************/
struct GL_Program {
	GL_Program();
	GL_Program(GL_Program&& other) noexcept = delete;

	GL_Program(const GL_Program&) = delete;
	auto operator=(const GL_Program&)->GL_Program & = delete;
	auto operator=(GL_Program&&)->GL_Program & = delete;

	~GL_Program();

	auto bind() const -> void;
	auto unbind() const -> void;
	auto attach(const GL_Shader& shader) const -> void;
	auto link() const -> void;
	auto detach(const GL_Shader& shader) const -> void;
	auto validate() const -> bool;

	auto get_uniform_block_index(const char* name) const -> GLuint;
	auto set_uniform_block_binding(GLuint index, GLuint binding_point) const -> void;


	auto get_info(GLenum pname) const->GLint;

	auto get_info_log(GLsizei max_length) const->std::string;

	// Introspection
	auto get_uniform_location(const std::string& name) const->GLint;
	auto get_attribute_location(const std::string& name) const->GLint;
public:
	GLuint m_ID;
private:
	auto release()->GLuint;
	auto reset(GLuint ID = 0) -> void;
};


/*******************
*	TEXTURE
*******************/
template<GLenum texture_type>
struct GL_Texture {
	GL_Texture(const GL_Texture&) = delete;
	auto operator=(const GL_Texture&) noexcept -> GL_Texture & = delete;
	auto operator=(GL_Texture&&) noexcept -> GL_Texture & = delete;

	GL_Texture() noexcept;
	~GL_Texture();
	GL_Texture(GL_Texture&& other) noexcept = delete;

	auto bind(u32 unit) const -> void;
	auto unbind() const -> void;
	auto generate_mipmap() const -> void;
	auto set_texture_parameters(GLenum pname, GLint param) const -> void;
	auto set_texture_image_1D(GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) const -> void;
	auto set_texture_image_2D(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) const -> void;
	auto set_texture_image_3D(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) const -> void;
	GLuint m_ID;

private:
	auto release()->GLuint;
	auto reset(GLuint ID = 0) -> void;
};


template<GLenum texture_type>
GL_Texture<texture_type>::GL_Texture() noexcept {
	glCreateTextures(texture_type, 1, &m_ID);
}
template<GLenum texture_type>
GL_Texture<texture_type>::~GL_Texture() {
	this->reset();
}
//template<GLenum texture_type>
//GL_Texture<texture_type>::GL_Texture(GL_Texture&& other) noexcept
//	: m_ID(other.release()) {
//}
template<GLenum texture_type>
auto GL_Texture<texture_type>::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}template<GLenum texture_type>
auto GL_Texture<texture_type>::reset(GLuint ID) -> void {
	glDeleteTextures(1, &m_ID);
	m_ID = ID;
}
template<GLenum texture_type>
auto GL_Texture<texture_type>::bind(u32 unit) const -> void {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(texture_type, m_ID);
}
template<GLenum texture_type>
auto GL_Texture<texture_type>::unbind() const -> void {
	glBindTexture(texture_type, 0);
}
template<GLenum texture_type>
auto GL_Texture<texture_type>::generate_mipmap() const -> void {
	glGenerateMipmap(texture_type);
}
template<GLenum texture_type>
auto GL_Texture<texture_type>::set_texture_parameters(GLenum pname, GLint param) const -> void {
	glTexParameteri(texture_type, pname, param);
}
template<GLenum texture_type>
auto GL_Texture<texture_type>::set_texture_image_1D(GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) const -> void {
	glTexImage1D(texture_type, level, internalformat, width, border, format, type, pixels);
}
template<GLenum texture_type>
auto GL_Texture<texture_type>::set_texture_image_2D(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) const -> void {
	glTexImage2D(texture_type, level, internalformat, width, height, border, format, type, pixels);
}
template<GLenum texture_type>
auto GL_Texture<texture_type>::set_texture_image_3D(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) const -> void {
	glTexImage3D(texture_type, level, internalformat, width, height, depth, border, format, type, pixels);
}
}