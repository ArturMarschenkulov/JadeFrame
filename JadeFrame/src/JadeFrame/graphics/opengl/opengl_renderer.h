#pragma once
#include "opengl_texture.h"
#include "opengl_buffer.h"
#include "opengl_shader.h"
#include "opengl_context.h"

#include "JadeFrame/graphics/camera.h"

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/graphics/material_handle.h"
#include "../graphics_shared.h"

#include <vector>
#include <stack>
#include <map>

#ifdef _WIN32
#include "JadeFrame/platform/windows/windows_window.h"
#elif __linux__
#include "JadeFrame/platform/linux/linux_window.h"
#endif

namespace JadeFrame {

enum class PRIMITIVE_TYPE {
    TRIANGLES = GL_TRIANGLES,
    LINES = GL_LINES,
    POINTS = GL_POINTS,
};

struct OpenGL_Material {
    const opengl::Texture* m_texture = nullptr;
    opengl::Shader*        m_shader = nullptr;
};




struct OpenGL_RenderCommand {
    const Matrix4x4*         transform = nullptr;
    const VertexData*        vertex_data = nullptr;
    MaterialHandle*          material_handle = nullptr;
    const GPUDataMeshHandle* m_GPU_mesh_data = nullptr;
};
class OpenGL_CommandBuffer {
public:
    // auto push(const Mesh& mesh, const OpenGL_Material& material, const Matrix4x4& tranform, const OpenGL_VertexArray&
    // vertex_array) -> void;
    std::vector<OpenGL_RenderCommand> m_render_commands;
};


class OpenGL_Renderer : public IRenderer {
public:
    OpenGL_Renderer(const IWindow* window);

    virtual auto present() -> void override;
    virtual auto clear_background() -> void override;
    virtual auto render(const Matrix4x4& view_projection) -> void override;

    virtual auto submit(const Object& obj) -> void override;

    virtual auto set_clear_color(const RGBAColor& color) -> void override;
    virtual auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void override;


    virtual auto take_screenshot(const char* filename) -> void override;




private:
    auto render_mesh(const opengl::GPUMeshData* buffer_data, const VertexData* vertex_data) const -> void;

private:
    OpenGL_Context                           m_context;
    mutable std::deque<OpenGL_RenderCommand> m_render_commands;

    struct FB {
        Object                      m_fb;
        OGLW_Texture<GL_TEXTURE_2D> m_framebuffer_texture;
        OGLW_Renderbuffer           m_framebuffer_renderbuffer;
        OGLW_Framebuffer            m_framebuffer;
        opengl::GPUMeshData*        m_framebuffer_rect;
        ShaderHandle*               m_shader_handle_fb;
    } fb;
};
static_assert(is_renderer<OpenGL_Renderer>);

struct RenderSystem {};
} // namespace JadeFrame