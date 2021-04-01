#pragma once
#include "opengl_texture.h"
#include "opengl_buffer.h"
#include "opengl_shader.h"
#include "opengl_context.h"

#include "../camera.h"

#include "../../math/vec_3.h"
#include "../../math/vec_2.h"
#include "../../math/mat_4.h"
#include "../mesh.h"

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

struct GLBufferData {
	OpenGL_VertexArray vertex_array;

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
	OpenGL_Shader* m_shader = nullptr;
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