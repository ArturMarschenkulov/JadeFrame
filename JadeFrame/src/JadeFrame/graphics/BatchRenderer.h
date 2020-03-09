#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>


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
			}
			else {
				glDisable(GL_DEPTH_TEST);
			}
		}
	}
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

		GLuint VBO = 0;
		GLuint VAO = 0;
		GLuint IBO = 0;

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
	auto ortho(float left, float right, float buttom, float top, float zNear, float zFar) -> void;
	auto perspective(float fovy, float aspect, float zNear, float zFar) -> void;
	auto translate(Vec3 vec) -> void;
	auto rotate(float angle, float x, float y, float z) -> void;
	auto scale(float x, float y, float z) -> void;

	auto push_matrix() -> void { matrix_stack.push(); }
	auto pop_matrix() -> void { matrix_stack.pop(); }
	//private:
	struct MatrixStack {
		std::stack<Mat4> stack;
		Mat4 model_matrix;
		Mat4 view_matrix;
		Mat4 projection_matrix;
		Mat4* current_matrix;
		Mat4 transform_matrix;
		bool use_transform_matrix;
	public:
		auto push() -> void {
			use_transform_matrix = true;
			current_matrix = &transform_matrix;
			stack.push(*current_matrix);
		}
		auto pop() -> void {
			if (!stack.empty()) {
				Mat4 mat = stack.top();
				*current_matrix = mat;
				stack.pop();
			}
			if (stack.empty()) {
				current_matrix = &view_matrix;
				use_transform_matrix = false;
			}
		}
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
	//void set_clear_color(const Color& color);
	//void set_depth_test(bool enable);
};