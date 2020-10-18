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


enum BLENDING_FACTOR : int {
	ZERO = GL_ZERO, 
	ONE = GL_ONE, 
	SRC_COLOR = GL_SRC_COLOR,
	ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR, 
	DST_COLOR = GL_DST_COLOR, 
	ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR, 
	SRC_ALPHA = GL_SRC_ALPHA, 
	ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
	DST_ALPHA = GL_DST_ALPHA,
	ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
	CONSTANT_COLOR = GL_CONSTANT_COLOR, 
	ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR, 
	CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
	ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
};


struct GLCache {

	bool depth_test;
	Color clear_color;
	GLbitfield clear_bitfield;
	bool blending;

	auto set_blending(bool enable, BLENDING_FACTOR sfactor = SRC_ALPHA, BLENDING_FACTOR dfactor = ONE_MINUS_SRC_ALPHA) -> void {
		if (blending != enable) {
			blending = enable;
			if (enable) {
				glEnable(GL_BLEND);
				glBlendFunc(sfactor, dfactor);
			} else {
				glDisable(GL_BLEND);
			}
		}
	}

	auto set_clear_color(const Color& color) -> void {
		if (clear_color != color) {
			clear_color = color;
			glClearColor(color.r, color.g, color.b, color.a);
		}
	}

	auto set_clear_bitfield(const GLbitfield& bitfield) -> void {
		clear_bitfield = bitfield;
	}
	auto add_clear_bitfield(const GLbitfield& bitfield) -> void {
		clear_bitfield |= (1 << bitfield);
	}
	auto remove_clear_bitfield(const GLbitfield& bitfield) -> void {
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
		std::cout << "glvb d" << std::endl;
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
		int vertex_buffer_index = 0;
		for (int i = 0; i != buffer_layout.m_elements.size(); i++) {
			BufferElement& element = buffer_layout.m_elements[i];
			switch (element.type) {
			case SHADER_DATA_TYPE::FLOAT:
			case SHADER_DATA_TYPE::FLOAT_2:
			case SHADER_DATA_TYPE::FLOAT_3:
			case SHADER_DATA_TYPE::FLOAT_4:
			{
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
	auto set_layout2(BufferLayout buffer_layout) const -> void {

		int vertex_buffer_index = 0;
		for (auto& element : buffer_layout) {
			switch (element.type) {
			case SHADER_DATA_TYPE::FLOAT:
			case SHADER_DATA_TYPE::FLOAT_2:
			case SHADER_DATA_TYPE::FLOAT_3:
			case SHADER_DATA_TYPE::FLOAT_4:
			{
				bind();
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
	PRIMITIVE_TYPE m_primitive_type = PRIMITIVE_TYPE::TRIANGLES;


	GLBufferData() {
	}
	auto init(Mesh mesh) -> void {
		//CPU
		vertices.resize(mesh.vertices.size());
		for (GLuint i = 0; i < mesh.vertices.size(); i++) {
			vertices[i + vertex_offset].position = mesh.vertices[i].position;
			vertices[i + vertex_offset].color = current_color;
			vertex_count++;
		}
		indices.resize(mesh.indices.size());
		for (int i = 0; i < mesh.indices.size(); i++) {
			indices[i + index_offset] = mesh.indices[i] + vertex_offset;
			index_count++;
		}

		//GPU
		vertex_buffer.bind();
		vertex_buffer.send_to_GPU(vertices.size() * sizeof(Vertex), vertices.data());

		vertex_array.bind();
		BufferLayout layout = {
			{SHADER_DATA_TYPE::FLOAT_3, "v_pos"},
			{SHADER_DATA_TYPE::FLOAT_4, "v_col"},
			{SHADER_DATA_TYPE::FLOAT_2, "v_texture_coord"},
		};
		vertex_array.set_layout(layout);

		index_buffer.bind();
		index_buffer.send_to_GPU(indices.size() * sizeof(GLuint), indices.data());

		vertex_array.unbind();

	}
	auto add_to_buffer(const Mesh& mesh) -> void {
		for (GLuint i = 0; i < mesh.vertices.size(); i++) {
			vertices[i + vertex_offset].position = mesh.vertices[i].position;
			vertices[i + vertex_offset].color = current_color;
			vertex_count++;
		}

		for (int i = 0; i < mesh.indices.size(); i++) {
			indices[i + index_offset] = mesh.indices[i] + vertex_offset;
			index_count++;
		}

		vertex_offset += mesh.vertices.size();
		index_offset += mesh.indices.size();
	}
	auto update() -> void {
		vertex_array.bind();
		vertex_buffer.bind();
		vertex_buffer.update(/*vertex_buffer_size_in_bytes*/vertices.size() * sizeof(Vertex), vertices.data());
		index_buffer.bind();
		index_buffer.update(/*index_buffer_size_in_bytes*/indices.size() * sizeof(GLuint), indices.data());
		vertex_array.unbind();
	}
	auto draw() -> void {
		vertex_array.bind();
		if (index_count > 0) {
			glDrawElements(static_cast<GLenum>(m_primitive_type), index_count, GL_UNSIGNED_INT, 0);
		} else {
			//__debugbreak();
		}
	}
	auto reset_counters() -> void {
		vertex_offset = 0;
		index_offset = 0;
		vertex_count = 0;
		index_count = 0;
	}
	auto set_color(const Color& color) -> void {
		current_color = color;
	}
};

struct GLBatchBufferData {
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
	PRIMITIVE_TYPE m_primitive_type = PRIMITIVE_TYPE::TRIANGLES;

	int MAX_BATCH_QUADS = 100000;
	int MAX_VERTICES_FOR_BATCH = 4 * MAX_BATCH_QUADS;
	int MAX_INDICES_FOR_BATCH = 6 * MAX_BATCH_QUADS;

	auto init() -> void {
		//CPU
		vertices.resize(MAX_VERTICES_FOR_BATCH);
		indices.resize(MAX_INDICES_FOR_BATCH);

		//GPU
		vertex_buffer.bind();
		vertex_buffer.reserve_in_GPU(/*vertex_buffer_size_in_bytes*/vertices.size() * sizeof(Vertex));

		vertex_array.bind();
		BufferLayout layout = {
			{SHADER_DATA_TYPE::FLOAT_3, "v_pos"},
			{SHADER_DATA_TYPE::FLOAT_4, "v_col"},
			{SHADER_DATA_TYPE::FLOAT_2, "v_texture_coord"},
		};
		vertex_array.set_layout(layout);

		index_buffer.bind();
		index_buffer.reserve_in_GPU(/*index_buffer_size_in_bytes*/indices.size() * sizeof(GLuint));

		vertex_array.unbind();
	}
	auto add_to_buffer(const Mesh& mesh) -> void {
		//if vertex or index count is larger than the max, flush the data
		if ((vertex_count + mesh.vertices.size() > MAX_VERTICES_FOR_BATCH) ||
			(index_count + mesh.indices.size() > MAX_INDICES_FOR_BATCH)) {
			update();
			draw();
			reset_counters();
			__debugbreak();
		}

		for (GLuint i = 0; i < mesh.vertices.size(); i++) {
			vertices[i + vertex_offset].position = mesh.vertices[i].position;
			vertices[i + vertex_offset].color = current_color;
			vertex_count++;
		}

		for (int i = 0; i < mesh.indices.size(); i++) {
			indices[i + index_offset] = mesh.indices[i] + vertex_offset;
			index_count++;
		}


		if (0) {
			Mesh2 mesh;
			for (GLuint i = 0; i < mesh.positions.size(); i++) {
				vertices[i + vertex_offset].position = mesh.positions[i];
				vertices[i + vertex_offset].color = current_color;
				vertex_count++;
			}

			for (int i = 0; i < mesh.indices.size(); i++) {
				indices[i + index_offset] = mesh.indices[i] + vertex_offset;
				index_count++;
			}
		}
		vertex_offset += mesh.vertices.size();
		index_offset += mesh.indices.size();
	}
	auto update() -> void {
		vertex_array.bind();
		vertex_buffer.bind();
		vertex_buffer.update(/*vertex_buffer_size_in_bytes*/vertex_count * sizeof(Vertex), vertices.data());
		index_buffer.bind();
		index_buffer.update(/*index_buffer_size_in_bytes*/index_count * sizeof(GLuint), indices.data());
		vertex_array.unbind();
	}
	auto draw() -> void {
		vertex_array.bind();
		if (index_count > 0) {
			glDrawElements(static_cast<GLenum>(m_primitive_type), index_count, GL_UNSIGNED_INT, 0);
		} else {
			//__debugbreak();
		}
	}
	auto reset_counters() -> void {
		vertex_offset = 0;
		index_offset = 0;
		vertex_count = 0;
		index_count = 0;
	}
	auto set_color(const Color& color) -> void {
		current_color = color;
	}
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


	auto draw_mesh(Mesh mesh) -> void {

	}

	GLBatchBufferData buffer_data;
	GLShader* current_shader = nullptr;
	MatrixStack matrix_stack;
	Camera cam;
	GLCache gl_cache;
};