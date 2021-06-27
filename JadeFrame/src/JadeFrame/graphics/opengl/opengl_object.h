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

namespace JadeFrame {

class GL_VertexBuffer {
public:
	GL_VertexBuffer();
	~GL_VertexBuffer();
	GL_VertexBuffer(GL_VertexBuffer&& other) noexcept;

	GL_VertexBuffer(const GL_VertexBuffer&) = delete;
	auto operator=(const GL_VertexBuffer&)->GL_VertexBuffer & = delete;
	auto operator=(GL_VertexBuffer&&)->GL_VertexBuffer & = delete;

	auto release() -> GLuint {
		GLuint ret = m_ID;
		m_ID = 0;
		return ret;
	}
	auto reset(GLuint ID = 0) -> void {
		glDeleteBuffers(1, &m_ID);
		m_ID = ID;
	}

	auto bind() const -> void;
	auto unbind() const -> void;
	auto reserve(GLuint size_in_bytes) const -> void;
	auto send(const std::vector<f32>& data) const -> void;
	auto update(GLuint size_in_bytes, const void* data) const -> void;
private:
	GLuint m_ID;
};
class GL_VertexArray {
public:
	GL_VertexArray();
	~GL_VertexArray();
	GL_VertexArray(GL_VertexArray&& other) noexcept;

	GL_VertexArray(GL_VertexArray&) = delete;
	auto operator=(const GL_VertexArray&)->GL_VertexArray & = delete;
	auto operator=(GL_VertexArray&&)->GL_VertexArray & = delete;

	auto release() -> GLuint {
		GLuint ret = m_ID;
		m_ID = 0;
		return ret;
	}
	auto reset(GLuint ID = 0) -> void {
		glDeleteBuffers(1, &m_ID);
		m_ID = ID;
	}

	auto bind() const -> void;
	auto unbind() const -> void;
private:
	GLuint m_ID;
};

class GL_IndexBuffer {
public:
	GL_IndexBuffer();
	~GL_IndexBuffer();
	GL_IndexBuffer(GL_IndexBuffer&& other) noexcept;

	GL_IndexBuffer(GL_IndexBuffer&) = delete;
	auto operator=(const GL_IndexBuffer&)->GL_IndexBuffer & = delete;
	auto operator=(GL_IndexBuffer&&)->GL_IndexBuffer & = delete;

	auto release() -> GLuint {
		GLuint ret = m_ID;
		m_ID = 0;
		return ret;
	}
	auto reset(GLuint ID = 0) -> void {
		glDeleteBuffers(1, &m_ID);
		m_ID = ID;
	}

	auto bind() const -> void;
	auto unbind() const -> void;
	auto reserve(GLuint size_in_bytes) const -> void;
	auto send(const std::vector<GLuint>& data) const -> void;
	auto update(GLuint size_in_bytes, const GLuint* data) const -> void;
private:
	GLuint m_ID;
};

struct GL_Shader {
	GL_Shader() = default;
	GL_Shader(GL_Shader&& other) noexcept;
	GL_Shader(const GLenum type);
	GL_Shader(const GLenum type, const std::string& source_code);
	~GL_Shader();

	GL_Shader(const GL_Shader&) = delete;
	auto operator=(const GL_Shader&)->GL_Shader & = delete;
	auto operator=(GL_Shader&&)->GL_Shader & = delete;

	auto release()->GLuint;
	auto reset(GLuint ID = 0) -> void;

	auto set_source(const std::string& source_code) -> void;
	auto compile() -> void;
	auto get_info(GLenum pname)->GLint;
	auto get_compile_status()->GLint;
	auto get_info_log(GLsizei max_length)->std::string;
public:
	enum class SHADER_TYPE {

	};
	SHADER_TYPE m_type;
	GLuint m_ID;
};
struct GL_Program {
	GL_Program();
	GL_Program(GL_Program&& other) noexcept;

	GL_Program(const GL_Program&) = delete;
	auto operator=(const GL_Program&)->GL_Program & = delete;
	auto operator=(GL_Program&&)->GL_Program & = delete;

	~GL_Program();
	auto release()->GLuint;
	auto reset(GLuint ID = 0) -> void;
	auto bind() const -> void;
	auto unbind() const -> void;
	auto attach(const GL_Shader& shader) const -> void;
	auto link() const -> void;
	auto detach(const GL_Shader& shader) const -> void;
	auto validate() const -> bool;

	auto get_uniform_location(const std::string& name) const->GLint;
	auto get_attribute_location(const std::string& name) const->GLint;
	auto get_info(GLenum pname) const->GLint;

	auto get_info_log(GLsizei max_length) const->std::string;
public:
	GLuint m_ID;
};


struct GL_Texture {
	GL_Texture() noexcept;
	~GL_Texture();
	GL_Texture(GL_Texture&& other) noexcept;
	GL_Texture(const GL_Texture&) = delete;
	auto operator=(const GL_Texture&) noexcept -> GL_Texture & = delete;
	auto operator=(GL_Texture&&) noexcept -> GL_Texture & = delete;
	auto release()->GLuint;
	auto reset(GLuint ID = 0) -> void;
	auto bind(GLenum target) const -> void;
	auto unbind(GLenum target) const -> void;
	auto generate_mipmap(GLenum target) const -> void;
	auto set_texture_parameters(GLenum target, GLenum pname, GLint param) const -> void;
	auto set_texture_image_1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels) const -> void;
	auto set_texture_image_2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) const -> void;
	auto set_texture_image_3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) const -> void;
	GLuint m_ID;
};
}