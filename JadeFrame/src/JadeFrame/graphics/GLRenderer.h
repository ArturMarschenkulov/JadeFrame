#pragma once


#include "GLShader.h"
#include "Camera.h"


#include <vector>
#include "../math/Vec3.h"
#include "../math/Vec2.h"
#include "../math/Mat4.h"
#include "Mesh.h"
#include <stack>
#include <stb\stb_image.h>


enum class PRIMITIVE_TYPE {
	TRIANGLES = GL_TRIANGLES,
	LINES = GL_LINES
};

enum class SHADER_DATA_TYPE {
	NONE = 0,
	FLOAT, FLOAT_2, FLOAT_3, FLOAT_4,
	MAT_3, MAT_4,
	INT, INT_2, INT_3, INT_4,
	BOOL
};

static auto shader_data_type_to_openGL_base_type(const SHADER_DATA_TYPE type) -> GLenum {
	switch (type) {
	case SHADER_DATA_TYPE::FLOAT:
	case SHADER_DATA_TYPE::FLOAT_2:
	case SHADER_DATA_TYPE::FLOAT_3:
	case SHADER_DATA_TYPE::FLOAT_4: return GL_FLOAT;
	default: __debugbreak(); return 0;
	}
}
static auto shader_data_type_size(const SHADER_DATA_TYPE type) -> uint32_t {
	switch (type) {
	case SHADER_DATA_TYPE::FLOAT:	return 4;
	case SHADER_DATA_TYPE::FLOAT_2:	return 4 * 2;
	case SHADER_DATA_TYPE::FLOAT_3:	return 4 * 3;
	case SHADER_DATA_TYPE::FLOAT_4:	return 4 * 4;
	case SHADER_DATA_TYPE::MAT_3:	return 4 * 3 * 3;
	case SHADER_DATA_TYPE::MAT_4:	return 4 * 4 * 4;
	case SHADER_DATA_TYPE::INT:		return 4;
	case SHADER_DATA_TYPE::INT_2:	return 4 * 2;
	case SHADER_DATA_TYPE::INT_3:	return 4 * 3;
	case SHADER_DATA_TYPE::INT_4:	return 4 * 4;
	case SHADER_DATA_TYPE::BOOL:	return 1;
	default: __debugbreak(); return -1;
	}
	return 0;
}


struct BufferElement {
	std::string name;
	SHADER_DATA_TYPE type;
	uint32_t size;
	size_t offset;
	bool normalized;

	BufferElement(SHADER_DATA_TYPE type, const std::string& name, bool normalized = false)
		: name(name), type(type), size(shader_data_type_size(type)), offset(0), normalized(normalized) {
	}

	auto get_component_count() -> uint32_t {
		switch (type) {
		case SHADER_DATA_TYPE::FLOAT:	return 1;
		case SHADER_DATA_TYPE::FLOAT_2:	return 2;
		case SHADER_DATA_TYPE::FLOAT_3:	return 3;
		case SHADER_DATA_TYPE::FLOAT_4:	return 4;
		case SHADER_DATA_TYPE::MAT_3:	return 3; // 3* float3
		case SHADER_DATA_TYPE::MAT_4:	return 4; // 4* float4
		case SHADER_DATA_TYPE::INT:		return 1;
		case SHADER_DATA_TYPE::INT_2:	return 2;
		case SHADER_DATA_TYPE::INT_3:	return 3;
		case SHADER_DATA_TYPE::INT_4:	return 4;
		case SHADER_DATA_TYPE::BOOL:	return 1;
		default: __debugbreak(); return -1;
		}
	}
};
class BufferLayout {
public:
	BufferLayout(const std::initializer_list<BufferElement>& elements)
		: m_elements(elements) {
		calculate_offset_and_stride();
	}

	std::vector<BufferElement>::iterator begin() {
		return m_elements.begin();
	}
	std::vector<BufferElement>::iterator end() {
		return m_elements.end();
	}
	std::vector<BufferElement>::const_iterator begin() const {
		return m_elements.begin();
	}
	std::vector<BufferElement>::const_iterator end() const {
		return m_elements.end();
	}

	void calculate_offset_and_stride() {
		size_t offset = 0;
		m_stride = 0;
		for (auto& element : m_elements) {
			element.offset = offset;
			offset += element.size;
			m_stride += element.size;
		}
	}

	std::vector<BufferElement> m_elements;
	uint32_t m_stride = 0;

};


struct GLCache {

	bool depth_test;
	Color clear_color;
	GLbitfield clear_bitfield;

	auto set_clear_color(const Color& color) -> void {
		if (clear_color != color) {
			clear_color = color;
			glClearColor(color.r, color.g, color.b, color.a);
		}
	}


	auto add_clear_bitfield(GLbitfield bitfield) -> void {
		clear_bitfield |= (1 << bitfield);
	}
	auto remove_clear_bitfield(GLbitfield bitfield) -> void {
		clear_bitfield &= ~(1 << bitfield);
	}
	auto set_depth_test(bool enable) -> void {
		if (depth_test != enable) {
			depth_test = enable;
			if (enable) {
				glEnable(GL_DEPTH_TEST);
			} else {
				glDisable(GL_DEPTH_TEST);
			}
		}
	}
};

class GLTexture {
public:
	GLTexture() {
		glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
	}
	~GLTexture() {
		glDeleteTextures(1, &m_texture);
	}
	auto bind() -> void {
		glBindTexture(GL_TEXTURE_2D, m_texture);
	}

	auto generate() -> void {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int num_channels;
		unsigned char* data = stbi_load("", &m_width, &m_height, &num_channels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else {
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	}
	GLuint m_texture;
	int m_width;
	int m_height;
};

class GLVertexBuffer {
public:
	GLVertexBuffer() {
		glCreateBuffers(1, &m_ID);
	}
	~GLVertexBuffer() {
		glDeleteBuffers(1, &m_ID);
	}

	auto bind() const -> void {
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	}
	auto reserve_in_GPU(GLuint size_in_bytes) const -> void {
		// if NULL is passed in as data, it only reserves size_in_bytes bytes.
		glBufferData(GL_ARRAY_BUFFER, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
	}
	auto send_to_GPU(GLuint size_in_bytes, Vertex* data) const -> void {
		glBufferData(GL_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
	}
	auto update(GLuint size_in_bytes, Vertex* data) const -> void {
		glBufferSubData(GL_ARRAY_BUFFER, 0, size_in_bytes, data);
	}
	GLuint m_ID = 0;
};
class GLVertexArray {
public:
	GLVertexArray() {
		glCreateVertexArrays(1, &m_ID);
	}
	~GLVertexArray() {
		glDeleteVertexArrays(1, &m_ID);
	}
	auto bind() const -> void {
		glBindVertexArray(m_ID);
	}
	auto unbind() const -> void {
		glBindVertexArray(0);
	}
	auto set_layout() const -> void {
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(1);
	}

	auto set_layout(BufferLayout buffer_layout) const -> void {

		for(auto& element : buffer_layout) {
			switch(element.type) {
			case SHADER_DATA_TYPE::FLOAT:
			case SHADER_DATA_TYPE::FLOAT_2:
			case SHADER_DATA_TYPE::FLOAT_3:
			case SHADER_DATA_TYPE::FLOAT_4:
			{
				static int vertex_buffer_index = 0;
				glEnableVertexAttribArray(vertex_buffer_index);
				glVertexAttribPointer(
					vertex_buffer_index,
					element.get_component_count(),
					shader_data_type_to_openGL_base_type(element.type),
					element.normalized ? GL_TRUE : GL_FALSE,
					buffer_layout.m_stride,
					//reinterpret_cast<const void*>(element.offset)
					(const void*)element.offset
				);
				vertex_buffer_index++;
			} break;
			default: __debugbreak();

			}
		}
	}
	GLuint m_ID = 0;
};
class GLIndexBuffer {
public:
	GLIndexBuffer() {
		glCreateBuffers(1, &m_ID);
	}
	~GLIndexBuffer() {
		glDeleteBuffers(1, &m_ID);
	}
	auto bind() const -> void {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	}
	auto reserve_in_GPU(GLuint size_in_bytes) const -> void {
		// if NULL is passed in as data, it only reserves size_in_bytes bytes.
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
	}
	auto send_to_GPU(GLuint size_in_bytes, GLuint* data) const -> void {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
	}
	auto update(GLuint size_in_bytes, GLuint* data) const -> void {
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size_in_bytes, data);
	}
	GLuint m_ID = 0;
};

struct GLBufferData {
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	GLVertexBuffer vertex_buffer;
	GLVertexArray vertex_array;
	GLIndexBuffer index_buffer;

	GLuint vertex_offset = 0;
	GLuint index_offset = 0;

	GLuint vertex_count = 0;
	GLuint index_count = 0;

	Color current_color = { 0.5f, 0.5f, 0.5f, 1.0f };
	PRIMITIVE_TYPE m_primitive_type;

	int MAX_BATCH_QUADS = 100000;
	int MAX_VERTICES_FOR_BATCH = 4 * MAX_BATCH_QUADS;
	int MAX_INDICES_FOR_BATCH = 6 * MAX_BATCH_QUADS;

	auto init() -> void;
	auto add_to_buffer(const Mesh& mesh) -> void;
	auto update() -> void;
	auto draw() -> void;
	auto reset_counters() -> void;
	auto set_color(const Color& color) -> void;
};

struct MatrixStack {
	std::stack<Mat4> stack;
	Mat4 model_matrix = Mat4();
	Mat4 view_matrix = Mat4();
	Mat4 projection_matrix = Mat4();
};

class GLRenderer {
public:
	GLRenderer();
	auto init() -> void;
	auto start(PRIMITIVE_TYPE type) -> void;
	auto end() -> void;

	GLBufferData buffer_data;
	GLShader* current_shader = nullptr;
	MatrixStack matrix_stack;
	Camera cam;
	GLCache gl_cache;
};