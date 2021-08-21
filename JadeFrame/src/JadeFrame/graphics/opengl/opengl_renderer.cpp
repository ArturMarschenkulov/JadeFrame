#include "pch.h"
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

auto OpenGL_Renderer::clear_background() -> void {
	auto bitfield = m_context.m_cache.clear_bitfield;
	glClear(bitfield);
}
auto OpenGL_Renderer::set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void {
	m_context.m_cache.set_viewport(x, y, width, height);

	//__debugbreak();
}

OpenGL_Renderer::OpenGL_Renderer(const Windows_Window& window)
	: m_context(window) {

	m_framebuffer.bind();

	const Vec2 size = m_context.m_cache.viewport[1];
	m_framebuffer_texture.bind(0);

	m_framebuffer_texture.set_texture_image_2D(0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	m_framebuffer_texture.set_texture_parameters(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_framebuffer_texture.set_texture_parameters(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_framebuffer_texture.set_texture_parameters(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_framebuffer_texture.set_texture_parameters(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_framebuffer.attach_texture_2D(m_framebuffer_texture);

	m_framebuffer_renderbuffer.bind();
	m_framebuffer_renderbuffer.store(GL_DEPTH24_STENCIL8, size.x, size.y);
	m_framebuffer.attach_renderbuffer(m_framebuffer_renderbuffer);

	m_framebuffer.unbind();

	const GLenum res = m_framebuffer.check_status();
	if (res != GL_FRAMEBUFFER_COMPLETE) __debugbreak();

	BufferLayout bl = {
			{ SHADER_TYPE::FLOAT_3, "v_position" },
			{ SHADER_TYPE::FLOAT_4, "v_color" },
			{ SHADER_TYPE::FLOAT_2, "v_texture_coord" },
			{ SHADER_TYPE::FLOAT_3, "v_normal" },
	};
	m_framebuffer_rect = new OpenGL_GPUMeshData(VertexDataFactory::make_rectangle({ -1.0f, -1.0f, 0.0f }, { 2.0f, 2.0f, 0.0f }), bl);

	m_shader_handle_fb = new ShaderHandle(GLSLCodeLoader::get_by_name("framebuffer_test"));
	m_shader_handle_fb->init();
}

auto OpenGL_Renderer::present() -> void {
	::SwapBuffers(m_context.m_device_context); // TODO: This is Windows specific. Abstract his away!
}
auto OpenGL_Renderer::submit(const Object& obj) -> void {

	if (obj.m_GPU_mesh_data.m_is_initialized == false) {
		//obj.m_GPU_mesh_data.m_handle = new OpenGL_GPUMeshData();
		//static_cast<OpenGL_GPUMeshData*>(obj.m_GPU_mesh_data.m_handle)->finalize(*obj.m_mesh);

		const BufferLayout bl = {
			{ SHADER_TYPE::FLOAT_3, "v_position" },
			{ SHADER_TYPE::FLOAT_4, "v_color" },
			{ SHADER_TYPE::FLOAT_2, "v_texture_coord" },
			{ SHADER_TYPE::FLOAT_3, "v_normal" },
		};
		obj.m_GPU_mesh_data.m_handle = new OpenGL_GPUMeshData(*obj.m_mesh, bl);
		obj.m_GPU_mesh_data.m_is_initialized = true;
	}
	if (obj.m_material_handle->m_is_initialized == false) {
		obj.m_material_handle->init();
		obj.m_material_handle->m_is_initialized = true;
	}


	const OpenGL_RenderCommand command = {
		.transform = &obj.m_transform,
		.mesh = obj.m_mesh,
		.material_handle = obj.m_material_handle,
		.m_GPU_mesh_data = &obj.m_GPU_mesh_data,
	};
	m_render_commands.push_back(command);
}


auto OpenGL_Renderer::render(const Matrix4x4& view_projection) -> void {
#define JF_FB 1
#if JF_FB
	m_framebuffer.bind();
#endif

	this->clear_background();
	for (size_t i = 0; i < m_render_commands.size(); ++i) {

		OpenGL_Shader& shader = *static_cast<OpenGL_Shader*>	(m_render_commands[i].material_handle->m_shader_handle->m_handle);
		shader.bind();

		const Matrix4x4& transform = *m_render_commands[i].transform;
		const std::vector<Matrix4x4>& u = { view_projection , transform };
		m_context.m_uniform_buffers[0].bind();
		m_context.m_uniform_buffers[0].send(u);
		m_context.m_uniform_buffers[0].unbind();

		OpenGL_Texture& texture = *static_cast<OpenGL_Texture*>	(m_render_commands[i].material_handle->m_texture_handle->m_handle);
		texture.bind();


		const Mesh* mesh = m_render_commands[i].mesh;

		const OpenGL_GPUMeshData* vertex_array = static_cast<OpenGL_GPUMeshData*>(m_render_commands[i].m_GPU_mesh_data->m_handle);
		this->render_mesh(vertex_array, mesh);
	}
#if JF_FB
	m_framebuffer.unbind();
	static_cast<OpenGL_Shader*>(m_shader_handle_fb->m_handle)->bind();
	m_framebuffer_texture.bind(0);
	m_framebuffer_rect->m_vertex_array.bind();
	m_context.m_cache.set_depth_test(false);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	m_context.m_cache.set_depth_test(true);
#endif
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
		Logger::log("data failed");
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

auto OpenGL_Renderer::main_loop() -> void {
	// dummy
}
}