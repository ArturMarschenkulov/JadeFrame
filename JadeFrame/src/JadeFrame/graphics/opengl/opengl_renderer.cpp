#include "opengl_renderer.h"
#include "JadeFrame/base_app.h"

#include "Windows.h"

#include <cassert>


auto OpenGL_Renderer::set_clear_color(const Color& color) -> void {
	m_context.gl_cache.set_clear_color(color);
}

auto OpenGL_Renderer::clear_background() const -> void {
	auto bitfield = m_context.gl_cache.clear_bitfield;
	glClear(bitfield);
}
auto OpenGL_Renderer::set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void {
	glViewport(x, y, width, height);
}

auto OpenGL_Renderer::swap_buffer(const HWND window_handle) const -> void {
	::SwapBuffers(GetDC(window_handle)); // TODO: This is Windows specific. Abstract his away!
}
auto OpenGL_Renderer::push_to_renderer(const Object& obj) -> void {
	RenderCommand command = {};
	command.mesh = obj.m_mesh;
	command.material = obj.m_material;
	command.transform = &obj.m_transform;
	command.vertex_array = &obj.m_vertex_array;
	m_render_commands.push_back(command);
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

	for (size_t i = 0; i < m_render_commands.size(); ++i) {

		const Material* material = m_render_commands[i].material;
		const Mesh* mesh = m_render_commands[i].mesh;
		const Matrix4x4* transform = m_render_commands[i].transform;
		const OpenGL_VertexArray* vertex_array = m_render_commands[i].vertex_array;

		material->m_shader->bind();
		material->m_shader->set_uniform("u_view_projection", view_projection);
		material->m_shader->set_uniform("u_model", *transform);
		material->m_texture->bind();

		this->render_mesh(vertex_array, mesh);
	}
}
auto OpenGL_Renderer::clear_render_commands() -> void {
	m_render_commands.clear();
}
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "extern/stb/stb_image_write.h"
#include <chrono>
#include <thread>
#include <future>

auto OpenGL_Renderer::take_screenshot(const char* filename) -> void {
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	i32 x = viewport[0];
	i32 y = viewport[1];
	i32 width = viewport[2];
	i32 height = viewport[3];

	char* data = (char*)malloc((size_t)(width * height * 3));
	if (!data) {
		std::cout << "data failed" << std::endl;
		return;
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

	auto c = [](const char* filename, i32 width, i32 height, char* data) {
		stbi_write_png(filename, width, height, 3, data, 0);
		free(data);
	};
	std::thread t(c, filename, width, height, data);
	t.detach();
}

auto OpenGL_Renderer::set_context(const HWND& window_handle) -> void {
	m_context = OpenGL_Context(window_handle);
}

auto CommandBuffer::push(const Mesh& mesh, const Material& material, const Matrix4x4& tranform, const OpenGL_VertexArray& vertex_array) -> void {
	RenderCommand command = {};
	command.mesh = &mesh;
	command.material = &material;
	command.transform = &tranform;
	command.vertex_array = &vertex_array;

	m_render_commands.push_back(command);
}
