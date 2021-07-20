#include "opengl_renderer.h"
#include "JadeFrame/base_app.h"

#include "Windows.h"

#include <cassert>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "extern/stb/stb_image_write.h"
#include <chrono>
#include <thread>
#include <future>

namespace JadeFrame {

auto OpenGL_Renderer::set_clear_color(const Color& color) -> void {
	m_context.m_cache.set_clear_color(color);
}

auto OpenGL_Renderer::clear_background() const -> void {
	auto bitfield = m_context.m_cache.clear_bitfield;
	glClear(bitfield);
}
auto OpenGL_Renderer::set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void {
	glViewport(x, y, width, height);
}

auto OpenGL_Renderer::swap_buffer(const HWND window_handle) const -> void {
	::SwapBuffers(GetDC(window_handle)); // TODO: This is Windows specific. Abstract his away!
}
auto OpenGL_Renderer::submit(const Object& obj) -> void {


	if (obj.m_GPU_mesh_data.m_is_initialized == false) {


		obj.m_GPU_mesh_data.m_handle = new OpenGL_GPUMeshData();
		static_cast<OpenGL_GPUMeshData*>(obj.m_GPU_mesh_data.m_handle)->finalize(*obj.m_mesh);
		obj.m_GPU_mesh_data.m_is_initialized = true;
		//s->finalize(*obj.m_mesh); // NOTE: OpenGL specific
	}
	if (obj.m_material_handle->m_is_initialized == false) {
		obj.m_material_handle->init();
		obj.m_material_handle->m_is_initialized = true;
	}


	OpenGL_RenderCommand command = {
		.transform = &obj.m_transform,
		.mesh = obj.m_mesh,
		.material_handle = obj.m_material_handle,
		.m_GPU_mesh_data = &obj.m_GPU_mesh_data,
	};
	m_render_commands.push_back(command);
}


auto OpenGL_Renderer::render(const Matrix4x4& view_projection) const -> void {

	for (size_t i = 0; i < m_render_commands.size(); ++i) {
		const Matrix4x4* transform = m_render_commands[i].transform;

		MaterialHandle* material_handle = m_render_commands[i].material_handle;
		OpenGL_Shader* shader = static_cast<OpenGL_Shader*>(material_handle->m_shader_handle->m_handle);
		OpenGL_Texture* texture = static_cast<OpenGL_Texture*>(material_handle->m_texture_handle->m_handle);

		shader->bind();
		const std::vector<Matrix4x4> u = { view_projection , *transform };
		shader->set_uniform_block("UniformBufferObject", u);

		texture->bind();

		const Mesh* mesh = m_render_commands[i].mesh;
		const OpenGL_GPUMeshData* vertex_array = static_cast<OpenGL_GPUMeshData*>(m_render_commands[i].m_GPU_mesh_data->m_handle);

		this->render_mesh(vertex_array, mesh);
	}
	m_render_commands.clear();
}

auto OpenGL_Renderer::render_mesh(const OpenGL_GPUMeshData* vertex_array, const Mesh* mesh) const -> void {
	vertex_array->bind();

	if (mesh->m_indices.size() > 0) {
		const GLenum mode = static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES);
		const GLsizei count = mesh->m_indices.size();
		const GLenum type = GL_UNSIGNED_INT;
		const void* indices = nullptr;
		glDrawElements(mode, count, type, indices);
	} else {
		const GLenum mode = static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES);
		const GLint first = 0;
		const GLsizei count = mesh->m_positions.size();
		glDrawArrays(mode, first, count);
	}
}


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

auto OpenGL_Renderer::set_context(const Windows_Window& window_handle) -> void {
	m_context = OpenGL_Context(window_handle);
}
}