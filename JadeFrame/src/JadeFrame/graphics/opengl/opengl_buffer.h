#pragma once
#include <glad/glad.h>

#include <cstdint>
#include <string>
#include <vector>
struct Vertex;

//TODO: Consider whether to keep this wrapper
enum BUFFER_USAGE : GLenum {
	STREAM_DRAW = GL_STREAM_DRAW,
	STREAM_READ = GL_STREAM_READ,
	STREAM_COPY = GL_STREAM_COPY,
	STATIC_DRAW = GL_STATIC_DRAW,
	STATIC_READ = GL_STATIC_READ,
	STATIC_COPY = GL_STATIC_COPY,
	DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
	DYNAMIC_READ = GL_DYNAMIC_READ,
	DYNAMIC_COPY = GL_DYNAMIC_COPY,
};

enum class SHADER_DATA_TYPE {
	NONE = 0,
	FLOAT, FLOAT_2, FLOAT_3, FLOAT_4,
	MAT_3, MAT_4,
	INT, INT_2, INT_3, INT_4,
	BOOL,
	SAMPLER_1D, SAMPLER_2D, SAMPLER_3D, SAMPLER_CUBE,
};
auto SHADER_DATA_TYPE_to_openGL_type(const SHADER_DATA_TYPE type)->GLenum;
auto SHADER_TYPE_from_openGL_enum(const GLenum type)->SHADER_DATA_TYPE;


class BufferLayout {
public:
	struct BufferElement {
		std::string name;
		SHADER_DATA_TYPE type;
		uint32_t size;
		size_t offset;
		bool normalized;

		BufferElement(SHADER_DATA_TYPE type, const std::string& name, bool normalized = false);
		//auto get_component_count() const->uint32_t;
	};

public:
	BufferLayout() = default;
	BufferLayout(const std::initializer_list<BufferElement>& elements);
	auto calculate_offset_and_stride() -> void;

	std::vector<BufferElement> m_elements;
	uint32_t m_stride = 0;

};

class GLVertexBuffer {
public:
	GLVertexBuffer();
	~GLVertexBuffer();
	GLVertexBuffer(const GLVertexBuffer&) = delete;
	GLVertexBuffer(GLVertexBuffer&& other);
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
	auto reserve_in_GPU(GLuint size_in_bytes) const -> void;
	auto send_to_GPU(const std::vector<float>& data) const -> void;
	auto update(GLuint size_in_bytes, const void* data) const -> void;
private:
	GLuint m_ID = 0;
	//BUFFER_USAGE buffer_usage = BUFFER_USAGE::STATIC_DRAW;
};
class GLVertexArray {
public:
	GLVertexArray();
	~GLVertexArray();
	GLVertexArray(GLVertexArray&) = delete;
	GLVertexArray(GLVertexArray&& other);

	auto operator=(const GLVertexArray&) -> GLVertexArray & = delete;
	auto operator=(GLVertexArray&&) -> GLVertexArray & = delete;
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
	auto set_layout(const BufferLayout& buffer_layout) -> void;
private:
	GLuint m_ID = 0;
	BufferLayout m_buffer_layout;
};
class GLIndexBuffer {
public:
	GLIndexBuffer();
	~GLIndexBuffer();
	GLIndexBuffer(GLIndexBuffer&) = delete;
	GLIndexBuffer(GLIndexBuffer&& other);
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
	auto reserve_in_GPU(GLuint size_in_bytes) const -> void;
	auto send_to_GPU(const std::vector<GLuint>& data) const -> void;
	auto update(GLuint size_in_bytes, const GLuint* data) const -> void;
private:
	GLuint m_ID = 0;
};