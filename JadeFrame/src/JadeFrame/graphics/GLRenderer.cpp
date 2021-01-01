#include "GLRenderer.h"
#include "../BaseApp.h"

auto GLRenderer::render(const Object& obj) -> void {
	if (obj.m_mesh == nullptr) {
		//__debugbreak();
		return;
	}
	obj.m_shader->bind();
	const auto& app = BaseApp::get_instance();
	auto& m_renderer = app->m_renderer;
	auto& m_camera = app->m_camera;

	m_renderer.matrix_stack.view_matrix = m_camera.get_view_matrix();
	Mat4 view_projection = m_renderer.matrix_stack.view_matrix * m_renderer.matrix_stack.projection_matrix;
	obj.m_shader->set_uniform_matrix("view_projection", view_projection);

	obj.m_shader->set_uniform_matrix("model", obj.m_transform);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, obj.m_texture->m_ID);

	obj.m_buffer_data.vertex_array.bind();

	if (obj.m_mesh->m_indices.size() > 0) {
		GLenum mode = static_cast<GLenum>(obj.m_mesh->m_primitive_type);
		GLsizei count = obj.m_mesh->m_indices.size();
		GLenum type = GL_UNSIGNED_INT;
		const void* indices = 0;

		glDrawElements(mode, count, type, indices);
	} else {
		GLenum mode = static_cast<GLenum>(obj.m_mesh->m_primitive_type);
		GLint first = 0;
		GLsizei count = obj.m_mesh->m_positions.size();

		glDrawArrays(mode, first, count);
		__debugbreak();
	}
}
