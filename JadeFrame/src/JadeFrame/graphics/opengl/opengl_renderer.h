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
#include "JadeFrame/graphics/shared.h"
#include "JadeFrame/graphics/material_handle.h"

#include <vector>
#include <stack>
#include <map>


#include "JadeFrame/platform/windows/windows_window.h"

namespace JadeFrame {

enum class PRIMITIVE_TYPE {
	TRIANGLES = GL_TRIANGLES,
	LINES = GL_LINES,
	POINTS = GL_POINTS,
};

struct OpenGL_Material {
	const OpenGL_Texture* m_texture = nullptr;
	OpenGL_Shader* m_shader = nullptr;
};

struct GPUDataMeshHandle {
	enum class API {
		UNDEFINED,
		OPENGL,
		VULKAN,
	} api = API::OPENGL;
	void* m_handle = nullptr;

	mutable bool m_is_initialized = false;

};

struct Object {
	Mesh* m_mesh;
	MaterialHandle* m_material_handle;
	Matrix4x4 m_transform;
	mutable GPUDataMeshHandle m_GPU_mesh_data;

	auto set_color(const Color& color) -> void {
		m_mesh->current_color = color;
	}
};


struct OpenGL_RenderCommand {
	const Matrix4x4* transform = nullptr;
	const Mesh* mesh = nullptr;
	MaterialHandle* material_handle = nullptr;
	const GPUDataMeshHandle* m_GPU_mesh_data = nullptr;
};
class OpenGL_CommandBuffer {
public:
	//auto push(const Mesh& mesh, const OpenGL_Material& material, const Matrix4x4& tranform, const OpenGL_VertexArray& vertex_array) -> void;
	std::vector<OpenGL_RenderCommand> m_render_commands;
};


class OpenGL_Renderer : public IRenderer {
public:

	OpenGL_Renderer(const Windows_Window& window);

	virtual auto present() const -> void override;
	virtual auto clear_background() const -> void override;
	virtual auto render(const Matrix4x4& view_projection) const -> void override;

	auto init(const Windows_Window& window) -> void;
	auto submit(const Object& obj) -> void;

	auto set_clear_color(const Color& color) -> void;
	auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void;


	auto take_screenshot(const char* filename) -> void;



private:
	auto render_mesh(const OpenGL_GPUMeshData* buffer_data, const Mesh* mesh) const -> void;

private:
	OpenGL_Context m_context;
	mutable std::deque<OpenGL_RenderCommand> m_render_commands;


	OGLW_Texture<GL_TEXTURE_2D> m_framebuffer_texture;
	OGLW_Renderbuffer m_framebuffer_renderbuffer;
	OGLW_Framebuffer m_framebuffer;

	OpenGL_GPUMeshData* m_framebuffer_rect;
	ShaderHandle* m_shader_handle_fb;
	
};
struct RenderSystem {

};
}