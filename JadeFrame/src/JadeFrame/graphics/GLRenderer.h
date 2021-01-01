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
#include "GLBuffer.h"

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

struct MatrixStack {
	std::stack<Mat4> stack;
	Mat4 model_matrix = Mat4();
	Mat4 view_matrix = Mat4();
	Mat4 projection_matrix = Mat4();
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
	auto set_color(Color color) -> void {
		m_mesh->set_color(color);
	}
};
class BaseApp;
class GLRenderer {
public:
	MatrixStack matrix_stack;
	Camera* m_current_camera;
	GLCache gl_cache;

	auto render(const Object& obj) -> void;

};