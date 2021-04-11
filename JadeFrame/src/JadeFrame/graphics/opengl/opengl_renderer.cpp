#include "opengl_renderer.h"
#include "Windows.h"
#include <cassert>

auto OpenGL_Renderer::clear(const GLbitfield bitfield) const -> void {
	glClear(bitfield);
}
auto OpenGL_Renderer::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const -> void {
	glViewport(x, y, width, height);
}

auto OpenGL_Renderer::swap_buffer(const HWND window_handle) const -> void {
	::SwapBuffers(GetDC(window_handle));
}
auto OpenGL_Renderer::push_to_renderer(const Object& obj) -> void {

	const Material* material = obj.m_material;
	const Mesh* mesh = obj.m_mesh;
	const Matrix4x4* transform = &obj.m_transform;
	const OpenGL_VertexArray* vertex_array = &obj.m_vertex_array;

	m_command_buffer.push(mesh, material, transform, vertex_array);
}

auto OpenGL_Renderer::render_mesh(const OpenGL_VertexArray* vertex_array, const Mesh* mesh) const -> void {
	vertex_array->bind();
	if (mesh->m_indices.size() > 0) {
		GLenum mode = static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES);
		GLsizei count = mesh->m_indices.size();
		GLenum type = GL_UNSIGNED_INT;
		const void* indices = nullptr;
		glDrawElements(mode, count, type, indices);
	} else {
		GLenum mode = static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES);
		GLint first = 0;
		GLsizei count = mesh->m_positions.size();
		glDrawArrays(mode, first, count);
	}
}
auto OpenGL_Renderer::render_pushed(const Matrix4x4& view_projection) const -> void {
	const std::vector<RenderCommand>& render_commands = m_command_buffer.m_render_commands;

	for (size_t i = 0; i < render_commands.size(); ++i) {

		const Material* material = render_commands[i].material;
		const Mesh* mesh = render_commands[i].mesh;
		const Matrix4x4* transform = render_commands[i].transform;
		const OpenGL_VertexArray* vertex_array = render_commands[i].vertex_array;

		material->m_shader->bind();
		material->m_shader->set_uniform("view_projection", view_projection);
		material->m_shader->set_uniform("model", *transform);
		material->m_texture->bind();

		this->render_mesh(vertex_array, mesh);

	}

}