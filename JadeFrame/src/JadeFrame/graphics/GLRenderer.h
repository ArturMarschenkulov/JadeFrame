#pragma once
#include "GLTexture.h"

#include "GLShader.h"
#include "Camera.h"


#include <vector>
#include "../math/Vec3.h"
#include "../math/Vec2.h"
#include "../math/Mat4.h"
#include "Mesh.h"
#include <stack>

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


enum class POLYGON_FACE : GLenum {
	FRONT_AND_BACK = GL_FRONT_AND_BACK,
	FRONT = GL_FRONT,
	BACK = GL_BACK,
};
enum class POLYGON_MODE : GLenum {
	POINT = GL_POINT,
	LINE = GL_LINE,
	FILL = GL_FILL,
};

enum class PRIMITIVE_TYPE {
	TRIANGLES = GL_TRIANGLES,
	LINES = GL_LINES
};



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

	void calculate_offset_and_stride() {
		size_t offset = 0;
		m_stride = 0;
		for (BufferElement& element : m_elements) {
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
	bool blending;
	std::pair<POLYGON_FACE, POLYGON_MODE>  polygon_mode;

	auto set_default() -> void {
		set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
		set_depth_test(true);
		set_clear_bitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		set_blending(true);
		set_polygon_mode(POLYGON_FACE::FRONT_AND_BACK, POLYGON_MODE::FILL);
	}

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

	void set_polygon_mode(POLYGON_FACE face, POLYGON_MODE mode) {
		if ((polygon_mode.first != face) || (polygon_mode.second != mode)) {
			polygon_mode = {face, mode};

			glPolygonMode((GLenum)face, (GLenum)mode);
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



class GLVertexBuffer {
public:
	GLVertexBuffer() {
		this->construct();
	}
	~GLVertexBuffer() {
		if (is_invalid == false) {
			this->destruct();
		}
	}
	GLVertexBuffer(const GLVertexBuffer&) = delete;
	GLVertexBuffer(GLVertexBuffer&& other) : m_ID(std::move(other.m_ID)) {
		other.is_invalid = true;
	}
	auto operator=(const GLVertexBuffer&) -> GLVertexBuffer& = delete;
	auto operator=(GLVertexBuffer&&) -> GLVertexBuffer& = delete;
	auto construct() -> void {
		glCreateBuffers(1, &m_ID);
	}
	auto destruct() -> void {
		glDeleteBuffers(1, &m_ID);
	}
	
	auto bind() const -> void {
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	}
	auto unbind() const -> void {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	auto reserve_in_GPU(GLuint size_in_bytes) const -> void {
		// if NULL is passed in as data, it only reserves size_in_bytes bytes.
		//glBufferData(GL_ARRAY_BUFFER, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
		glNamedBufferData(m_ID, size_in_bytes, NULL, buffer_usage);
	}
	auto send_to_GPU(GLuint size_in_bytes, Vertex* data) const -> void {
		//glBufferData(GL_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
		glNamedBufferData(m_ID, size_in_bytes, data, buffer_usage);
	}
	auto send_to_GPU(GLuint size_in_bytes, void* data) const -> void {
		//glBufferData(GL_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
		glNamedBufferData(m_ID, size_in_bytes, data, buffer_usage);
	}
	auto update(GLuint size_in_bytes, Vertex* data) const -> void {
		//glBufferSubData(GL_ARRAY_BUFFER, 0, size_in_bytes, data);
		glNamedBufferSubData(m_ID, 0, size_in_bytes, data);
	}
	GLuint m_ID = 0;
	BUFFER_USAGE buffer_usage = BUFFER_USAGE::STATIC_DRAW;
	bool is_invalid = false;
};
class GLVertexArray {
public:
	GLVertexArray() {
		this->construct();
	}
	~GLVertexArray() {
		if (is_invalid == false) {
			this->destruct();
		}
	}
	GLVertexArray(GLVertexArray&) = delete;
	GLVertexArray(GLVertexArray&& other) : m_ID(std::move(other.m_ID)) {
		other.is_invalid = true;
	}
	auto operator=(const GLVertexArray&) -> GLVertexArray& = delete;
	auto operator=(GLVertexArray&&) -> GLVertexArray& = delete;
	auto construct() -> void {
		glCreateVertexArrays(1, &m_ID);
	}
	auto destruct() -> void {
		glDeleteVertexArrays(1, &m_ID);
	}
	
	auto bind() const -> void {
		glBindVertexArray(m_ID);
	}
	auto unbind() const -> void {
		glBindVertexArray(0);
	}
	auto set_layout(BufferLayout buffer_layout) const -> void {
		int vertex_buffer_index = 0;
		for (size_t i = 0; i != buffer_layout.m_elements.size(); i++) {
			BufferElement& element = buffer_layout.m_elements[i];
			switch (element.type) {
			case SHADER_DATA_TYPE::FLOAT:
			case SHADER_DATA_TYPE::FLOAT_2:
			case SHADER_DATA_TYPE::FLOAT_3:
			case SHADER_DATA_TYPE::FLOAT_4:
			{
				glEnableVertexAttribArray(vertex_buffer_index);
				//glEnableVertexArrayAttrib(m_ID, vertex_buffer_index);
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
	bool is_invalid = false;
};
class GLIndexBuffer {
public:
	GLIndexBuffer() {
		this->construct();
	}
	~GLIndexBuffer() {
		if (is_invalid == false) {
			this->destruct();
		}
	}
	GLIndexBuffer(GLIndexBuffer&) = delete;
	GLIndexBuffer(GLIndexBuffer&& other) : m_ID(std::move(other.m_ID)) {
		other.is_invalid = true;
	}
	auto operator=(const GLIndexBuffer&) -> GLIndexBuffer& = delete;
	auto operator=(GLIndexBuffer&&) -> GLIndexBuffer& = delete;
	auto construct() -> void {
		glCreateBuffers(1, &m_ID);
	}
	auto destruct() -> void {
		glDeleteBuffers(1, &m_ID);
	}

	auto bind() const -> void {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	}
	auto unbind() const -> void {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	auto reserve_in_GPU(GLuint size_in_bytes) const -> void {
		// if NULL is passed in as data, it only reserves size_in_bytes bytes.
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
		glNamedBufferData(m_ID, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
	}
	auto send_to_GPU(GLuint size_in_bytes, GLuint* data) const -> void {
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
		glNamedBufferData(m_ID, size_in_bytes, data, GL_DYNAMIC_DRAW);
	}
	auto send_to_GPU(GLuint size_in_bytes, const GLuint* data) const -> void {
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
		glNamedBufferData(m_ID, size_in_bytes, data, GL_DYNAMIC_DRAW);
	}
	auto update(GLuint size_in_bytes, GLuint* data) const -> void {
		//glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size_in_bytes, data);
		glNamedBufferSubData(m_ID, 0, size_in_bytes, data);
	}
	GLuint m_ID = 0;
	bool is_invalid = false;
};

//
//
//struct GLBufferData {
//	std::vector<Vertex> vertices;
//	std::vector<GLuint> indices;
//	Color current_color = { 0.5f, 0.5f, 0.5f, 1.0f };
//	PRIMITIVE_TYPE m_primitive_type = PRIMITIVE_TYPE::TRIANGLES;
//
//	GLVertexBuffer vertex_buffer;
//	GLVertexArray vertex_array;
//	GLIndexBuffer index_buffer;
//
//	auto add_to_data(const VertexData& vertex_data) -> void {
//		auto vertex_data_size = vertex_data.positions.size() + vertex_data.colors.size();
//		vertices.resize(vertex_data_size);
//		for (GLuint i = 0; i < vertex_data.positions.size(); i++) {
//			vertices[i].position = vertex_data.positions[i];
//			vertices[i].color = current_color;
//			vertices[i].tex_coord = vertex_data.tex_coords[i];
//		}
//		indices.resize(vertex_data.indices.size());
//		for (int i = 0; i < vertex_data.indices.size(); i++) {
//			indices[i] = vertex_data.indices[i];
//		}
//	}
//
//	auto send_to_GPU() {
//		vertex_buffer.bind();
//		vertex_buffer.send_to_GPU(vertices.size() * sizeof(Vertex), vertices.data());
//
//		vertex_array.bind();
//		BufferLayout layout = {
//			{SHADER_DATA_TYPE::FLOAT_3, "v_position"},
//			{SHADER_DATA_TYPE::FLOAT_4, "v_color"},
//			{SHADER_DATA_TYPE::FLOAT_2, "v_texture_coord"},
//		};
//		vertex_array.set_layout(layout);
//
//		index_buffer.bind();
//		index_buffer.send_to_GPU(indices.size() * sizeof(GLuint), indices.data());
//
//		vertex_array.unbind();
//	}
//	auto update() -> void {
//		vertex_array.bind();
//		vertex_buffer.bind();
//		vertex_buffer.update(/*vertex_buffer_size_in_bytes*/vertices.size() * sizeof(Vertex), vertices.data());
//		index_buffer.bind();
//		index_buffer.update(/*index_buffer_size_in_bytes*/indices.size() * sizeof(GLuint), indices.data());
//		vertex_array.unbind();
//	}
//	auto draw() -> void {
//		vertex_array.bind();
//		if (indices.size() > 0) {
//			glDrawElements(static_cast<GLenum>(m_primitive_type), indices.size(), GL_UNSIGNED_INT, 0);
//		} else {
//			__debugbreak();
//		}
//	}
//	auto set_color(const Color& color) -> void {
//		current_color = color;
//	}
//};

//struct GLBatchBufferData {
//	std::vector<Vertex> vertices;
//	std::vector<GLuint> indices;
//
//	GLVertexBuffer vertex_buffer;
//	GLVertexArray vertex_array;
//	GLIndexBuffer index_buffer;
//
//	GLuint vertex_offset = 0;
//	GLuint index_offset = 0;
//
//	GLuint vertex_count = 0;
//	GLuint index_count = 0;
//
//	Color current_color = { 0.5f, 0.5f, 0.5f, 1.0f };
//	PRIMITIVE_TYPE m_primitive_type = PRIMITIVE_TYPE::TRIANGLES;
//
//	int MAX_BATCH_QUADS = 100000;
//	int MAX_VERTICES_FOR_BATCH = 4 * MAX_BATCH_QUADS;
//	int MAX_INDICES_FOR_BATCH = 6 * MAX_BATCH_QUADS;
//
//	auto init() -> void {
//		//CPU
//		vertices.resize(MAX_VERTICES_FOR_BATCH);
//		indices.resize(MAX_INDICES_FOR_BATCH);
//
//		//GPU
//		vertex_buffer.bind();
//		vertex_buffer.reserve_in_GPU(/*vertex_buffer_size_in_bytes*/vertices.size() * sizeof(Vertex));
//
//		vertex_array.bind();
//		BufferLayout layout = {
//			{SHADER_DATA_TYPE::FLOAT_3, "v_pos"},
//			{SHADER_DATA_TYPE::FLOAT_4, "v_col"},
//			{SHADER_DATA_TYPE::FLOAT_2, "v_texture_coord"},
//		};
//		vertex_array.set_layout(layout);
//
//		index_buffer.bind();
//		index_buffer.reserve_in_GPU(/*index_buffer_size_in_bytes*/indices.size() * sizeof(GLuint));
//
//		vertex_array.unbind();
//	}
//	auto add_to_buffer(const Mesh& mesh) -> void {
//		//if vertex or index count is larger than the max, flush the data
//		if ((vertex_count + mesh.vertices.size() > MAX_VERTICES_FOR_BATCH) ||
//			(index_count + mesh.indices.size() > MAX_INDICES_FOR_BATCH)) {
//			update();
//			draw();
//			reset_counters();
//			__debugbreak();
//		}
//
//		for (GLuint i = 0; i < mesh.vertices.size(); i++) {
//			vertices[i + vertex_offset].position = mesh.vertices[i].position;
//			vertices[i + vertex_offset].color = current_color;
//			vertex_count++;
//		}
//
//		for (int i = 0; i < mesh.indices.size(); i++) {
//			indices[i + index_offset] = mesh.indices[i] + vertex_offset;
//			index_count++;
//		}
//
//		vertex_offset += mesh.vertices.size();
//		index_offset += mesh.indices.size();
//	}
//	auto update() -> void {
//		vertex_array.bind();
//		vertex_buffer.bind();
//		vertex_buffer.update(/*vertex_buffer_size_in_bytes*/vertex_count * sizeof(Vertex), vertices.data());
//		index_buffer.bind();
//		index_buffer.update(/*index_buffer_size_in_bytes*/index_count * sizeof(GLuint), indices.data());
//		vertex_array.unbind();
//	}
//	auto draw() -> void {
//		vertex_array.bind();
//		if (index_count > 0) {
//			glDrawElements(static_cast<GLenum>(m_primitive_type), index_count, GL_UNSIGNED_INT, 0);
//		} else {
//			//__debugbreak();
//		}
//	}
//	auto reset_counters() -> void {
//		vertex_offset = 0;
//		index_offset = 0;
//		vertex_count = 0;
//		index_count = 0;
//	}
//	auto set_color(const Color& color) -> void {
//		current_color = color;
//	}
//};

//struct Object {
//	GLTexture* m_texture;
//	GLShader* m_shader;
//	Mat4 m_transform;
//	GLBufferData* m_buffer_data;
//
//	bool m_dirty;
//
//
//	auto draw() -> void {
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, m_texture->m_ID);
//		m_buffer_data->draw();
//	}
//	auto set_color(Color color) -> void {
//		m_buffer_data->set_color(color);
//	}
//};

struct MatrixStack {
	std::stack<Mat4> stack;
	Mat4 model_matrix = Mat4();
	Mat4 view_matrix = Mat4();
	Mat4 projection_matrix = Mat4();
};

class GLRenderer {
public:
	MatrixStack matrix_stack;
	Camera* m_current_camera;
	GLCache gl_cache;

};


struct Mesh {
	std::vector<Vec3>	m_positions;
	std::vector<Color>	m_colors;
	std::vector<Vec2>	m_tex_coords;
	std::vector<GLuint> m_indices;

	Color current_color = { 0.5f, 0.5f, 0.5f, 1.0f };
	PRIMITIVE_TYPE m_primitive_type = PRIMITIVE_TYPE::TRIANGLES;

	auto add_to_data(const VertexData& vertex_data) -> void {


		m_positions.resize(vertex_data.positions.size());
		m_colors.resize(vertex_data.positions.size());
		m_tex_coords.resize(vertex_data.tex_coords.size());
		for (GLuint i = 0; i < vertex_data.positions.size(); i++) {
			m_positions[i] = vertex_data.positions[i];
			m_colors[i] = current_color;
			m_tex_coords[i] = vertex_data.tex_coords[i];
		}

		m_indices.resize(vertex_data.indices.size());
		for (size_t i = 0; i < vertex_data.indices.size(); i++) {
			m_indices[i] = vertex_data.indices[i];
		}
	}
	auto set_color(const Color& color) -> void {
		current_color = color;
	}
};

struct GLBufferDataa {
	GLVertexBuffer vertex_buffer;
	GLVertexArray vertex_array;
	GLIndexBuffer index_buffer;
	PRIMITIVE_TYPE m_primitive_type = PRIMITIVE_TYPE::TRIANGLES;



	auto finalize(const Mesh& mesh, bool interleaved = true) -> void {
		std::vector<float> data;
		if (interleaved) {
			for (size_t i = 0; i < mesh.m_positions.size(); i++) {
				data.push_back(mesh.m_positions[i].x);
				data.push_back(mesh.m_positions[i].y);
				data.push_back(mesh.m_positions[i].z);
				if (mesh.m_colors.size()) {
					data.push_back(mesh.m_colors[i].r);
					data.push_back(mesh.m_colors[i].g);
					data.push_back(mesh.m_colors[i].b);
					data.push_back(mesh.m_colors[i].a);
				}
				if (mesh.m_tex_coords.size()) {
					data.push_back(mesh.m_tex_coords[i].x);
					data.push_back(mesh.m_tex_coords[i].y);
				}
			}
		} else {
			for (size_t i = 0; i < mesh.m_positions.size(); i++) {
				data.push_back(mesh.m_positions[i].x);
				data.push_back(mesh.m_positions[i].y);
				data.push_back(mesh.m_positions[i].z);
			}
			for (size_t i = 0; i < mesh.m_colors.size(); i++) {
				data.push_back(mesh.m_colors[i].r);
				data.push_back(mesh.m_colors[i].g);
				data.push_back(mesh.m_colors[i].b);
				data.push_back(mesh.m_colors[i].a);
			}
			for (size_t i = 0; i < mesh.m_tex_coords.size(); i++) {
				data.push_back(mesh.m_tex_coords[i].x);
				data.push_back(mesh.m_tex_coords[i].y);
			}
		}


		vertex_buffer.bind();
		vertex_buffer.send_to_GPU(data.size() * sizeof(float), data.data());

		vertex_array.bind();

		BufferLayout layout = {
			{SHADER_DATA_TYPE::FLOAT_3, "v_pos"},
			{SHADER_DATA_TYPE::FLOAT_4, "v_col"},
			{SHADER_DATA_TYPE::FLOAT_2, "v_texture_coord"},
		};
		vertex_array.set_layout(layout);
		if (0) {
			if (interleaved) {
				size_t stride = 0;
				if (mesh.m_positions.size() > 0) stride += 3 * sizeof(float);
				if (mesh.m_colors.size() > 0) stride += 4 * sizeof(float);
				if (mesh.m_tex_coords.size() > 0) stride += 2 * sizeof(float);

				size_t offset = 0;
				int vertex_buffer_index = 0;
				glEnableVertexAttribArray(vertex_buffer_index);
				glVertexAttribPointer(vertex_buffer_index, 3, GL_FLOAT, GL_FALSE, stride, (const void*)offset);
				offset += 3 * sizeof(float);
				vertex_buffer_index++;

				if (mesh.m_colors.size()) {
					glEnableVertexAttribArray(vertex_buffer_index);
					glVertexAttribPointer(vertex_buffer_index, 4, GL_FLOAT, GL_FALSE, stride, (const void*)offset);
					offset += 4 * sizeof(float);
					vertex_buffer_index++;
				}
				if (mesh.m_tex_coords.size()) {
					glEnableVertexAttribArray(vertex_buffer_index);
					glVertexAttribPointer(vertex_buffer_index, 2, GL_FLOAT, GL_FALSE, stride, (const void*)offset);
					offset += 2 * sizeof(float);
					vertex_buffer_index++;

				}
			} else {
				size_t offset = 0;
				int vertex_buffer_index = 0;
				glEnableVertexAttribArray(vertex_buffer_index);
				glVertexAttribPointer(vertex_buffer_index, 3, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
				vertex_buffer_index++;
				offset += mesh.m_positions.size() * sizeof(float);
				if (mesh.m_colors.size() > 0) {
					glEnableVertexAttribArray(vertex_buffer_index);
					glVertexAttribPointer(vertex_buffer_index, 4, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
					offset += mesh.m_colors.size() * sizeof(float);
					vertex_buffer_index++;
				}
				if (mesh.m_tex_coords.size() > 0) {
					glEnableVertexAttribArray(vertex_buffer_index);
					glVertexAttribPointer(vertex_buffer_index, 2, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
					offset += mesh.m_tex_coords.size() * sizeof(float);
					vertex_buffer_index++;
				}
			}
		}

		if (mesh.m_indices.size() > 0) {
			index_buffer.bind();
			index_buffer.send_to_GPU(mesh.m_indices.size() * sizeof(GLuint), mesh.m_indices.data());
		}
		vertex_array.unbind();
	}
};

struct Object {
	GLTexture* m_texture = nullptr;
	GLShader* m_shader = nullptr;
	Mat4 m_transform;
	GLBufferDataa m_buffer_data;
	Mesh* m_mesh = nullptr;
	//std::shared_ptr<Mesh> m_mesh;


	auto draw() -> void {
		if(m_mesh == nullptr) {
			//__debugbreak();
			return;
		}
		m_shader->set_uniform_matrix("model", m_transform);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture->m_ID);

		m_buffer_data.vertex_array.bind();

		if (m_mesh->m_indices.size() > 0) {
			GLenum mode = static_cast<GLenum>(m_mesh->m_primitive_type);
			GLsizei count = m_mesh->m_indices.size();
			GLenum type = GL_UNSIGNED_INT;
			const void* indices = 0;

			glDrawElements(mode, count, type, indices);
		} else {
			GLenum mode = static_cast<GLenum>(m_mesh->m_primitive_type);
			GLint first = 0;
			GLsizei count = m_mesh->m_positions.size();

			glDrawArrays(mode, first, count);
			//__debugbreak();
		}
	}
	auto set_color(Color color) -> void {
		m_mesh->set_color(color);
	}
};