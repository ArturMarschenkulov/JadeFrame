#pragma once
/*
	This is the place for the light OpenGL Wrapper classes.
	The goal is to make OpenGL objects more C++ like.
	This may or may not be temporary.

	These wrapper classes should have as little state as possible. The optimal would be to only have their ID and nothing more
*/
#include <glad/glad.h>
#include <vector>
#include <string>
#include <unordered_map>

class GLVertexBuffer {
public:
	GLVertexBuffer();
	~GLVertexBuffer();
	GLVertexBuffer(GLVertexBuffer&& other);

	GLVertexBuffer(const GLVertexBuffer&) = delete;
	auto operator=(const GLVertexBuffer&)->GLVertexBuffer & = delete;
	auto operator=(GLVertexBuffer&&)->GLVertexBuffer & = delete;

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
	auto send(const std::vector<float>& data) const -> void;
	auto update(GLuint size_in_bytes, const void* data) const -> void;
private:
	GLuint m_ID;
};
class GLVertexArray {
public:
	GLVertexArray();
	~GLVertexArray();
	GLVertexArray(GLVertexArray&& other);

	GLVertexArray(GLVertexArray&) = delete;
	auto operator=(const GLVertexArray&)->GLVertexArray & = delete;
	auto operator=(GLVertexArray&&)->GLVertexArray & = delete;

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

class GLIndexBuffer {
public:
	GLIndexBuffer();
	~GLIndexBuffer();
	GLIndexBuffer(GLIndexBuffer&& other);

	GLIndexBuffer(GLIndexBuffer&) = delete;
	auto operator=(const GLIndexBuffer&)->GLIndexBuffer & = delete;
	auto operator=(GLIndexBuffer&&)->GLIndexBuffer & = delete;

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

struct GLShader {
	GLShader() = default;
	GLShader(GLShader&& other);
	GLShader(const GLenum type);
	GLShader(const GLenum type, const std::string& source_code);
	~GLShader();

	GLShader(const GLShader&) = delete;
	auto operator=(const GLShader&)->GLShader & = delete;
	auto operator=(GLShader&&)->GLShader & = delete;

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
struct GLProgram {
	GLProgram();
	GLProgram(GLProgram&& other);

	GLProgram(const GLProgram&) = delete;
	auto operator=(const GLProgram&)->GLProgram & = delete;
	auto operator=(GLProgram&&)->GLProgram & = delete;

	~GLProgram();
	auto release()->GLuint;
	auto reset(GLuint ID = 0) -> void;
	auto bind() const -> void;
	auto unbind() const -> void;
	auto attach(const GLShader& shader) const -> void;
	auto link() const -> void;
	auto detach(const GLShader& shader) const -> void;
	auto validate() const -> bool;

	auto get_uniform_location(const std::string& name) const->GLint;
	auto get_attribute_location(const std::string& name) const->GLint;
	auto get_info(GLenum pname) const->GLint;

	auto get_info_log(GLsizei max_length) const->std::string;
public:
	GLuint m_ID;
};