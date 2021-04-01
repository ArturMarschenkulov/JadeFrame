#include "opengl_renderer.h"
#include "windows.h"
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

auto OpenGL_Renderer::render(const Object& obj) const -> void {
	if (obj.m_mesh == nullptr) {
		return;
	}
	obj.m_material->m_shader->bind();

	//matrix_stack.view_matrix = m_current_camera->get_view_matrix();
	//Mat4 view_projection = matrix_stack.view_matrix * matrix_stack.projection_matrix;
	const Mat4 view_matrix = m_current_camera->get_view_matrix();
	const Mat4 view_projection = view_matrix * matrix_stack.projection_matrix;
	obj.m_material->m_shader->set_uniform("view_projection", view_projection);
	obj.m_material->m_shader->set_uniform("model", obj.m_transform);
	obj.m_material->m_texture->bind();

	obj.m_buffer_data.vertex_array.bind();

	if (obj.m_mesh->m_indices.size() > 0) {
		GLenum mode = static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES);
		GLsizei count = obj.m_mesh->m_indices.size();
		GLenum type = GL_UNSIGNED_INT;
		const void* indices = nullptr;
		glDrawElements(mode, count, type, indices);
	} else {
		GLenum mode = static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES);
		GLint first = 0;
		GLsizei count = obj.m_mesh->m_positions.size();
		glDrawArrays(mode, first, count);
		//__debugbreak();
	}
}
static auto convert_into_data(const Mesh& mesh, const bool interleaved) -> std::vector<float> {
	//assert(mesh.m_positions.size() == mesh.m_normals.size());
	const uint64_t size
		= mesh.m_positions.size() * 3
		+ mesh.m_colors.size() * 4
		+ mesh.m_tex_coords.size() * 2
		+ mesh.m_normals.size() * 3;

	std::vector<float> data;
	data.reserve(size);
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
			if (mesh.m_normals.size()) {
				data.push_back(mesh.m_normals[i].x);
				data.push_back(mesh.m_normals[i].y);
				data.push_back(mesh.m_normals[i].z);
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

	return data;
}
auto GLBufferData::finalize(const Mesh& mesh, bool interleaved) -> void {
	const std::vector<float> data = convert_into_data(mesh, interleaved);

	vertex_array.foo(data, mesh);
	
}

