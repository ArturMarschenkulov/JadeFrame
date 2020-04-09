#pragma once


#include "Shader.h"
#include "Camera.h"


#include <vector>
#include "../math/Vec3.h"
#include "../math/Vec2.h"
#include "../math/Mat4.h"
#include "Mesh.h"
#include <stack>


enum class PRIMITIVE_TYPE {
	TRIANGLES = GL_TRIANGLES,
	LINES = GL_LINES
};

enum class EPrimitiveType {
	TRIANGLES = GL_TRIANGLES,
	LINES = GL_LINES
};



struct GLCache {

	bool depth_test;
	Color clear_color;

	auto set_clear_color(const Color& color) -> void {
		if (clear_color != color) {
			clear_color = color;
			glClearColor(color.r, color.g, color.b, color.a);
		}
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

class VertexBuffer {
public:
	VertexBuffer() {
		glCreateBuffers(1, &m_ID);
	}
	~VertexBuffer() {
		glDeleteBuffers(1, &m_ID);
	}

	auto bind() -> void {
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	}
	auto reserve_in_GPU(GLuint size_in_bytes) {
		// if NULL is passed in as data, it only reserves size_in_bytes bytes.
		glBufferData(GL_ARRAY_BUFFER, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
	}
	auto send_to_GPU(GLuint size_in_bytes, Vertex* data) {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
	}
	auto update(GLuint size_in_bytes, Vertex* data) -> void {
		glBufferSubData(GL_ARRAY_BUFFER, 0, size_in_bytes, data);
	}
	GLuint m_ID = 0;
};

class VertexArray {
public:
	VertexArray() {
		glCreateVertexArrays(1, &m_ID);
	}
	~VertexArray() {
		glDeleteVertexArrays(1, &m_ID);
	}
	auto bind() -> void {
		glBindVertexArray(m_ID);
	}
	auto unbind() -> void {
		glBindVertexArray(0);
	}
	auto set_layout() -> void {
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(1);
	}
	GLuint m_ID = 0;
};

class IndexBuffer {
public:
	IndexBuffer() {
		glCreateBuffers(1, &m_ID);
	}
	~IndexBuffer() {
		glDeleteBuffers(1, &m_ID);
	}
	auto bind() -> void {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	}
	auto reserve_in_GPU(GLuint size_in_bytes) {
		// if NULL is passed in as data, it only reserves size_in_bytes bytes.
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, NULL, GL_DYNAMIC_DRAW);
	}
	auto send_to_GPU(GLuint size_in_bytes, GLuint* data) {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_in_bytes, data, GL_DYNAMIC_DRAW);
	}
	auto update(GLuint size_in_bytes, GLuint* data) -> void {
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size_in_bytes, data);
	}
	GLuint m_ID = 0;
};
class BatchRenderer {
public:
	BatchRenderer();
	auto init(Shader* shader) -> void;
	auto start(PRIMITIVE_TYPE type) -> void;
	auto handle_mesh(Mesh& mesh) -> void;
	auto end() -> void;



	//private:
	struct BufferData {
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;

		VertexBuffer vertex_buffer;
		VertexArray vertex_array;
		IndexBuffer index_buffer;

		GLuint vertex_offset = 0;
		GLuint index_offset = 0;

		GLuint vertex_count = 0;
		GLuint index_count = 0;

		Color current_color = { 0.5f, 0.5f, 0.5f, 1.0f };
		PRIMITIVE_TYPE m_primitive_type;

		void init();
		void add_to_buffer(Mesh& mesh);
		void update();
		void draw();
		void reset_counters();
	};
	BufferData buffer_data;
	Shader* current_shader = nullptr;

	//Matrix operations
public:
	auto update_matrices_and_send_to_GPU() -> void;

	//private:
	struct MatrixStack {
		std::stack<Mat4> stack;
		Mat4 model_matrix = Mat4();
		Mat4 view_matrix;
		Mat4 projection_matrix;
	public:
		auto set_matrices_to_identity() -> void {
			projection_matrix = Mat4();
			view_matrix = Mat4();
			model_matrix = Mat4();
		}
	};
	MatrixStack matrix_stack;
	Camera cam;
	//Drawing API
public:
	GLCache gl_cache;
	auto set_color(const Color& color) -> void;
};