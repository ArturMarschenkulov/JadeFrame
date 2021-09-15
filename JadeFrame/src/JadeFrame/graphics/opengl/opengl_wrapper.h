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
#include <cassert>

namespace JadeFrame {

/*******************
*	BUFFER
*******************/
template<GLenum buffer_type>
class OGLW_Buffer {
public:

	OGLW_Buffer(const OGLW_Buffer<buffer_type>&) = delete;
	auto operator=(const OGLW_Buffer<buffer_type>&)->OGLW_Buffer<buffer_type> & = delete;
	auto operator=(OGLW_Buffer<buffer_type>&&)->OGLW_Buffer<buffer_type> & = delete;

	OGLW_Buffer();
	OGLW_Buffer(OGLW_Buffer<buffer_type>&& other) noexcept;
	~OGLW_Buffer();

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
auto OGLW_Buffer<buffer_type>::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}
template<GLenum buffer_type>
auto OGLW_Buffer<buffer_type>::reset(GLuint ID) -> void {
	glDeleteBuffers(1, &m_ID);
	m_ID = ID;
}

template<GLenum buffer_type>
OGLW_Buffer<buffer_type>::OGLW_Buffer() {
	//glGenBuffers(1, &m_ID);
	glCreateBuffers(1, &m_ID);
	glBindBuffer(buffer_type, m_ID);
}
template<GLenum buffer_type>
OGLW_Buffer<buffer_type>::OGLW_Buffer(OGLW_Buffer<buffer_type>&& other) noexcept 
	: m_ID(other.release()) {
}
template<GLenum buffer_type>
OGLW_Buffer<buffer_type>::~OGLW_Buffer() {
	this->reset();
}
template<GLenum buffer_type>
auto OGLW_Buffer<buffer_type>::bind() const -> void {
	//glBindBuffer(buffer_type, m_ID);
}
template<GLenum buffer_type>
auto OGLW_Buffer<buffer_type>::unbind() const -> void {
	//glBindBuffer(buffer_type, 0);
}
template<GLenum buffer_type>
auto OGLW_Buffer<buffer_type>::reserve(GLuint size_in_bytes) const -> void {
	// if NULL is passed in as data, it only reserves size_in_bytes bytes.
	//glBufferData(buffer_type, size_in_bytes, NULL, GL_STATIC_DRAW);
	glNamedBufferData(m_ID, size_in_bytes, NULL, GL_STATIC_DRAW);
}

template<GLenum buffer_type>
template<typename U>
auto OGLW_Buffer<buffer_type>::send(const std::vector<U>& data) const -> void {
	//glBufferData(buffer_type, data.size() * sizeof(U), data.data(), GL_STATIC_DRAW);
	glNamedBufferData(m_ID, data.size() * sizeof(U), data.data(), GL_STATIC_DRAW);
}
template<GLenum buffer_type>
auto OGLW_Buffer<buffer_type>::update(GLuint size_in_bytes, const void* data) const -> void {
	//glBufferSubData(buffer_type, 0, size_in_bytes, data);
	glNamedBufferSubData(m_ID, 0, size_in_bytes, data);
}
template<GLenum buffer_type>
auto OGLW_Buffer<buffer_type>::bind_base(GLuint binding_point) const -> void {
	assert(GL_UNIFORM_BUFFER == buffer_type);
	glBindBufferBase(buffer_type, binding_point, m_ID);
}
template<GLenum buffer_type>
auto OGLW_Buffer<buffer_type>::bind_buffer_range(GLuint index, GLintptr offset, GLsizeiptr size) const -> void {
	assert(GL_UNIFORM_BUFFER == buffer_type);
	glBindBufferRange(buffer_type, index, m_ID, offset, size);
}

/*******************
*	VERTEX ARRAY
*******************/
class OGLW_VertexArray {
public:
	OGLW_VertexArray();
	~OGLW_VertexArray();
	OGLW_VertexArray(OGLW_VertexArray&& other) noexcept;

	OGLW_VertexArray(OGLW_VertexArray&) = delete;
	auto operator=(const OGLW_VertexArray&)->OGLW_VertexArray & = delete;
	auto operator=(OGLW_VertexArray&&)->OGLW_VertexArray & = delete;



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

struct OGLW_Shader {
	OGLW_Shader() = default;
	OGLW_Shader(OGLW_Shader&& other) noexcept = delete;
	OGLW_Shader(const GLenum type);
	//OGLW_Shader(const GLenum type, const std::string& source_code);
	~OGLW_Shader();

	OGLW_Shader(const OGLW_Shader&) = delete;
	auto operator=(const OGLW_Shader&)->OGLW_Shader & = delete;
	auto operator=(OGLW_Shader&&)->OGLW_Shader & = delete;


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
struct OGLW_Program {
	OGLW_Program();
	OGLW_Program(OGLW_Program&& other) noexcept = delete;

	OGLW_Program(const OGLW_Program&) = delete;
	auto operator=(const OGLW_Program&)->OGLW_Program & = delete;
	auto operator=(OGLW_Program&&)->OGLW_Program & = delete;

	~OGLW_Program();

	auto bind() const -> void;
	auto unbind() const -> void;
	auto attach(const OGLW_Shader& shader) const -> void;
	auto link() const -> bool;
	auto detach(const OGLW_Shader& shader) const -> void;
	auto validate() const -> bool;

	auto get_uniform_block_index(const char* name) const -> GLuint;
	auto set_uniform_block_binding(GLuint index, GLuint binding_point) const -> void;


	auto get_info(GLenum pname) const->GLint;

	auto get_info_log() const->std::string;

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
struct OGLW_Texture {
	OGLW_Texture(const OGLW_Texture&) = delete;
	auto operator=(const OGLW_Texture&) noexcept -> OGLW_Texture & = delete;
	auto operator=(OGLW_Texture&&) noexcept -> OGLW_Texture & = delete;

	OGLW_Texture() noexcept;
	~OGLW_Texture();
	OGLW_Texture(OGLW_Texture&& other) noexcept = delete;

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
OGLW_Texture<texture_type>::OGLW_Texture() noexcept {
	glCreateTextures(texture_type, 1, &m_ID);
}
template<GLenum texture_type>
OGLW_Texture<texture_type>::~OGLW_Texture() {
	this->reset();
}
//template<GLenum texture_type>
//OGLW_Texture<texture_type>::OGLW_Texture(OGLW_Texture&& other) noexcept
//	: m_ID(other.release()) {
//}
template<GLenum texture_type>
auto OGLW_Texture<texture_type>::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}template<GLenum texture_type>
auto OGLW_Texture<texture_type>::reset(GLuint ID) -> void {
	glDeleteTextures(1, &m_ID);
	m_ID = ID;
}
template<GLenum texture_type>
auto OGLW_Texture<texture_type>::bind(u32 unit) const -> void {
	//glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(texture_type, m_ID);
}
template<GLenum texture_type>
auto OGLW_Texture<texture_type>::unbind() const -> void {
	glBindTexture(texture_type, 0);
}
template<GLenum texture_type>
auto OGLW_Texture<texture_type>::generate_mipmap() const -> void {
	//glGenerateMipmap(texture_type);
	glGenerateTextureMipmap(m_ID);
}
template<GLenum texture_type>
auto OGLW_Texture<texture_type>::set_texture_parameters(GLenum pname, GLint param) const -> void {
	glTextureParameteri(m_ID, pname, param);
	//glTexParameteri(texture_type, pname, param);
}
template<GLenum texture_type>
auto OGLW_Texture<texture_type>::set_texture_image_1D(GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) const -> void {
	glTexImage1D(texture_type, level, internalformat, width, border, format, type, pixels);
}
template<GLenum texture_type>
auto OGLW_Texture<texture_type>::set_texture_image_2D(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) const -> void {
	glTexImage2D(texture_type, level, internalformat, width, height, border, format, type, pixels);
	//glTextureStorage2D(m_ID, level, internalformat, width, height);
}
template<GLenum texture_type>
auto OGLW_Texture<texture_type>::set_texture_image_3D(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) const -> void {
	glTexImage3D(texture_type, level, internalformat, width, height, depth, border, format, type, pixels);
}

/*******************
*	RENDERBUFFER
*******************/

class OGLW_Renderbuffer {
public:
	OGLW_Renderbuffer(const OGLW_Renderbuffer&) = delete;
	auto operator=(const OGLW_Renderbuffer&) noexcept -> OGLW_Renderbuffer & = delete;
	auto operator=(OGLW_Renderbuffer&&) noexcept -> OGLW_Renderbuffer & = delete;

	OGLW_Renderbuffer();
	~OGLW_Renderbuffer();

	auto store(GLenum internal_format, GLsizei width, GLsizei height) const -> void;

	auto bind() const -> void;
	auto unbind() const -> void;


	GLuint m_ID;
private:
	auto release()->GLuint;
	auto reset(GLuint ID = 0) -> void;
};


inline OGLW_Renderbuffer::OGLW_Renderbuffer() {
	glCreateRenderbuffers(1, &m_ID);

}

inline OGLW_Renderbuffer::~OGLW_Renderbuffer() {

}

inline auto OGLW_Renderbuffer::store(GLenum internal_format, GLsizei width, GLsizei height) const -> void {
	glNamedRenderbufferStorage(m_ID, internal_format, width, height);
}

inline auto OGLW_Renderbuffer::bind() const -> void {
	glBindRenderbuffer(GL_RENDERBUFFER, m_ID);
}

inline auto OGLW_Renderbuffer::unbind() const -> void {
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

inline auto OGLW_Renderbuffer::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}
inline auto OGLW_Renderbuffer::reset(GLuint ID) -> void {
	glDeleteFramebuffers(1, &m_ID);
	m_ID = ID;
}

/*******************
*	FRAMEBUFFER
*******************/

class OGLW_Framebuffer {
public:
	OGLW_Framebuffer(const OGLW_Framebuffer&) = delete;
	auto operator=(const OGLW_Framebuffer&) noexcept -> OGLW_Framebuffer & = delete;
	auto operator=(OGLW_Framebuffer&&) noexcept -> OGLW_Framebuffer & = delete;

	OGLW_Framebuffer();
	~OGLW_Framebuffer();

	auto attach_texture_2D(const OGLW_Texture<GL_TEXTURE_2D>& texture) const -> void;
	auto attach_renderbuffer(const OGLW_Renderbuffer& renderbuffer) const -> void;
	auto check_status() const -> GLenum;
	auto bind() const -> void;
	auto unbind() const -> void;


	GLuint m_ID;
private:
	auto release()->GLuint;
	auto reset(GLuint ID = 0) -> void;
};


inline OGLW_Framebuffer::OGLW_Framebuffer() {
	glCreateFramebuffers(1, &m_ID);

}
inline OGLW_Framebuffer::~OGLW_Framebuffer() {
	this->reset();
}
inline auto OGLW_Framebuffer::attach_texture_2D(const OGLW_Texture<GL_TEXTURE_2D>& texture) const -> void {
	glNamedFramebufferTexture(m_ID, GL_COLOR_ATTACHMENT0, texture.m_ID, 0);
}
inline auto OGLW_Framebuffer::attach_renderbuffer(const OGLW_Renderbuffer& renderbuffer) const -> void {
	glNamedFramebufferRenderbuffer(m_ID, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer.m_ID);
}
inline auto OGLW_Framebuffer::bind() const -> void {
	glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
}

inline auto OGLW_Framebuffer::unbind() const -> void {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline auto OGLW_Framebuffer::check_status() const -> GLenum {
	GLenum result = glCheckNamedFramebufferStatus(m_ID, GL_FRAMEBUFFER);
	return result;
}

inline auto OGLW_Framebuffer::release() -> GLuint {
	GLuint ret = m_ID;
	m_ID = 0;
	return ret;
}
inline auto OGLW_Framebuffer::reset(GLuint ID) -> void {
	glDeleteFramebuffers(1, &m_ID);
	m_ID = ID;
}

}