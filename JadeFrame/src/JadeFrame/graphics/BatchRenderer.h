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



class BatchRenderer {
public:
	BatchRenderer();
	void init(Shader* shader);
	void start();
	void handle_mesh(Mesh& mesh);
	void end();

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
	void update_matrices();
	void ortho(float left, float right, float buttom, float top, float zNear, float zFar);
	void perspective(float fovy, float aspect, float zNear, float zFar);
	void translate(Vec3 vec);
	void rotate(float angle, float x, float y, float z);
	void scale(float x, float y, float z);

	void push_matrix() { matrix_stack.push(); }
	void pop_matrix() { matrix_stack.pop(); }
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
		void push() {
			use_transform_matrix = true;
			current_matrix = &transform_matrix;
			stack.push(*current_matrix);
		}
		void pop() {
			if(!stack.empty()) {
				Mat4 mat = stack.top();
				*current_matrix = mat;
				stack.pop();
			}
			if(stack.empty()) {
				current_matrix = &view_matrix;
				use_transform_matrix = false;
			}
		}
	};
	MatrixStack matrix_stack;
	Camera cam;
	//Drawing API
public:
	void set_color(const Color& color);
	void set_clear_color(const Color& color);
	int num_draw_calls;
};