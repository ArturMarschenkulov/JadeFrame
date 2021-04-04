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

enum class SHADER_TYPE {
	NONE = 0,
	FLOAT, FLOAT_2, FLOAT_3, FLOAT_4,
	MAT_3, MAT_4,
	INT, INT_2, INT_3, INT_4,
	BOOL,
	SAMPLER_1D, SAMPLER_2D, SAMPLER_3D, SAMPLER_CUBE,
};

#include "../mesh.h"
#include "opengl_object.h"

class BufferLayout {
public:
	struct BufferElement {
		std::string name;
		SHADER_TYPE type;
		uint32_t size;
		size_t offset;
		bool normalized;

		BufferElement(SHADER_TYPE type, const std::string& name, bool normalized = false);
	};

public:
	BufferLayout() = default;
	BufferLayout(const std::initializer_list<BufferElement>& elements);
	auto calculate_offset_and_stride() -> void;

	std::vector<BufferElement> m_elements;
	uint32_t m_stride = 0;

};

class OpenGL_VertexArray {
public:

	OpenGL_VertexArray()
		: m_vertex_buffer()
		, m_vertex_array()
		, m_index_buffer() {

	}
	auto bind() const -> void {
		m_vertex_array.bind();
	}
	auto set_layout(const BufferLayout& buffer_layout) -> void;
	auto finalize(const Mesh& mesh, bool interleaved = true) -> void;
private:
	GLVertexBuffer m_vertex_buffer;
	GLVertexArray m_vertex_array;
	GLIndexBuffer m_index_buffer;
	BufferLayout m_buffer_layout;
};


class GLFrameBuffer {
	GLFrameBuffer() {
		glCreateFramebuffers(1, &m_ID);
	}
	auto bind() -> void {
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
	}
	GLuint m_ID;
};