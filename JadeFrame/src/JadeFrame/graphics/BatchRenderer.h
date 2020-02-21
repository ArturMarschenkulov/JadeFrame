#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "BatchShader.h"

#include <vector>
#include "../math/Vec3.h"
#include "../math/Vec2.h"
#include "../math/Mat4.h"
#include "Mesh.h"
#include <stack>
enum class CAMERA_MODE {
	PERSPECTIVE,
	ORTHOGRAPIC
};
class Camera {
public:

	Camera();
	~Camera();

	void set_mode(CAMERA_MODE mode);

	Mat4 get_view_matrix();

	void set_default_location();

	void move();
	void update();

private:
	Vec3 m_position;
	Vec3 m_rotation;
	Vec3 m_front;
	Vec3 m_up;
	Vec3 m_worldUp;
	Vec3 m_right;

	float m_speed;
	float m_fovy;

	float m_current_time;
	float m_previous_time;
	float m_delta_time;



	// Euler Angles
	float m_yaw;
	float m_pitch;
	float m_roll;


};

class BatchRenderer {
public:
	BatchRenderer();
	void init(BatchShader* shader);
	void start();
	void handle_mesh(Mesh& mesh);
	void end();

private:
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
		void add(Mesh& mesh);
		void update();
		void draw();
		void reset_counters();
	};
	BufferData buffer_data;
	BatchShader* current_shader = nullptr;

	//Matrix operations
public:
	//void updateMatrices();
	void update_matrices();
	void ortho(float left, float right, float buttom, float top, float zNear, float zFar);
	void perspective(float fovy, float aspect, float zNear, float zFar);
	void translate(float x, float y, float z);
	void rotate(float angle, float x, float y, float z);
	void scale(float x, float y, float z);

	void push_matrix() { matrix_stack.push(); }
	void pop_matrix() { matrix_stack.pop(); }
	//sprivate:
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

	//Drawing API
public:
	void set_color(const Color& color);
	void set_clear_color(const Color& color);
	int num_draw_calls;
};