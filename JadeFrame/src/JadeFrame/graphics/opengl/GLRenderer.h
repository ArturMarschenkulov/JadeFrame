#pragma once
#include "GLTexture.h"
#include "GLBuffer.h"
#include "GLShader.h"
#include "GLContext.h"

#include "../Camera.h"

#include "../../math/Vec3.h"
#include "../../math/Vec2.h"
#include "../../math/Mat4.h"
#include "../Mesh.h"

#include <vector>
#include <stack>
#include <map>
enum class PRIMITIVE_TYPE {
	TRIANGLES = GL_TRIANGLES,
	LINES = GL_LINES
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
	std::vector<Vec3> m_normals;
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

		m_normals.resize(vertex_data.normals.size());
		for (GLuint i = 0; i < vertex_data.normals.size(); i++) {
			m_normals[i] = vertex_data.normals[i];
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



struct GLBufferData {
	GLVertexBuffer vertex_buffer;
	GLVertexArray vertex_array;
	GLIndexBuffer index_buffer;
	PRIMITIVE_TYPE m_primitive_type = PRIMITIVE_TYPE::TRIANGLES;

	auto finalize(const Mesh& mesh, bool interleaved = true) -> void;
};


struct UniformValueSampler {
	SHADER_DATA_TYPE type;
	uint32_t unit;
	GLTexture* texture;
};
struct Material {
	const GLTexture* m_texture = nullptr;
	const OpenGL_Shader* m_shader = nullptr;
	std::map<std::string, UniformValueSampler> m_sampler_uniforms;
};
struct Object {
	Material* m_material = nullptr;
	Mesh* m_mesh = nullptr;

	Mat4 m_transform;
	GLBufferData m_buffer_data;
	//std::shared_ptr<Mesh> m_mesh;
	auto set_color(const Color& color) -> void {
		m_mesh->set_color(color);
	}
};


struct HWND__;	typedef HWND__* HWND;
struct HGLRC__; typedef HGLRC__* HGLRC;
struct HDC__;	typedef HDC__* HDC;

class OpenGL_Renderer {
public:
	auto swap_buffer(HWND window_handle) const -> void;
	auto clear(GLbitfield bitfield) const -> void;
	auto set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const -> void;
	auto render(const Object& obj) const -> void;
public:
	MatrixStack matrix_stack;
	Camera1* m_current_camera;
	GLContext gl_context;
};