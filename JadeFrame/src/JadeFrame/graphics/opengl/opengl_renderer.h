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

struct Material {
	const GLTexture* m_texture = nullptr;
	OpenGL_Shader* m_shader = nullptr;
};
struct Object {
	Material* m_material = nullptr;
	Mesh* m_mesh = nullptr;

	Mat4 m_transform;
	OpenGL_VertexArray m_vertex_array;

	auto set_color(const Color& color) -> void {
		m_mesh->current_color = color;
	}
};

struct RenderCommand {
	const Mat4* transform;
	const Mesh* mesh;
	const Material* material;
	const OpenGL_VertexArray* vertex_array;
};
class CommandBuffer {
public:
	auto push(const Mesh* mesh, const Material* material, const Mat4* tranform, const OpenGL_VertexArray* vertex_array) {
		RenderCommand command = {};
		command.mesh = mesh;
		command.material = material;
		command.transform = tranform;
		command.vertex_array = vertex_array;

		m_render_commands.push_back(command);
	}
	std::vector<RenderCommand> m_render_commands;
};

struct HWND__;	typedef HWND__* HWND;
class OpenGL_Renderer {
public:
	auto swap_buffer(HWND window_handle) const -> void;
	auto clear(GLbitfield bitfield) const -> void;
	auto set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const -> void;

	auto push_to_renderer(const Object& obj) -> void;
	auto render_mesh(const OpenGL_VertexArray* buffer_data, const Mesh* mesh) -> void;
	auto render_pushed(const Mat4& view_projection) -> void;
public:
	GLContext gl_context;
	CommandBuffer m_command_buffer;
};