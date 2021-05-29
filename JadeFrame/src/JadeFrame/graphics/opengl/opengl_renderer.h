#pragma once
#include "opengl_texture.h"
#include "opengl_buffer.h"
#include "opengl_shader.h"
#include "opengl_context.h"

#include "JadeFrame/graphics/camera.h"

#include "JadeFrame/math/vec_3.h"
#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/graphics/mesh.h"

#include <vector>
#include <stack>
#include <map>


enum class PRIMITIVE_TYPE {
	TRIANGLES = GL_TRIANGLES,
	LINES = GL_LINES,
	POINTS = GL_POINTS,
};

struct OpenGL_Material {
	const OpenGL_Texture* m_texture = nullptr;
	OpenGL_Shader* m_shader = nullptr;
};
struct Object {
	OpenGL_Material* m_material = nullptr;
	Mesh* m_mesh = nullptr;

	Matrix4x4 m_transform;
	OpenGL_VertexArray m_vertex_array;

	auto set_color(const Color& color) -> void {
		m_mesh->current_color = color;
	}
};

struct OpenGL_RenderCommand {
	const Matrix4x4* transform = nullptr;
	const Mesh* mesh = nullptr;
	const OpenGL_Material* material = nullptr;
	const OpenGL_VertexArray* vertex_array = nullptr;
};
class OpenGL_CommandBuffer {
public:
	auto push(const Mesh& mesh, const OpenGL_Material& material, const Matrix4x4& tranform, const OpenGL_VertexArray& vertex_array) -> void;
	std::vector<OpenGL_RenderCommand> m_render_commands;
};

struct HWND__;	typedef HWND__* HWND;
class OpenGL_Renderer {
public:
	auto swap_buffer(HWND window_handle) const -> void;
	auto set_clear_color(const Color& color) -> void;

	auto clear_background() const -> void;
	auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void;

	auto push_to_renderer(const Object& obj) -> void;
	auto render_mesh(const OpenGL_VertexArray* buffer_data, const Mesh* mesh) const -> void;
	auto render_pushed(const Matrix4x4& view_projection) const -> void;

	auto take_screenshot(const char* filename) -> void;

	auto set_context(const HWND& window) -> void;

private:
	OpenGL_Context m_context;
	mutable std::deque<OpenGL_RenderCommand> m_render_commands;
};