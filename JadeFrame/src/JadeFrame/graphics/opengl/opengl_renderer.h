#pragma once

#include "JadeFrame/graphics/camera.h"
#ifdef _WIN32
    #include "JadeFrame/platform/windows/windows_window.h"
#elif __linux__
    #include "JadeFrame/platform/linux/linux_window.h"
#endif

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/graphics/graphics_shared.h"

#include "opengl_texture.h"
#include "opengl_buffer.h"
#include "opengl_shader.h"
#include "opengl_context.h"

namespace JadeFrame {

#define JF_OPENGL_FB 1

enum class PRIMITIVE_TYPE {
    TRIANGLES = GL_TRIANGLES,
    LINES = GL_LINES,
    POINTS = GL_POINTS,
};

struct OpenGL_Material {
    const opengl::Texture* m_texture = nullptr;
    opengl::Shader*        m_shader = nullptr;
};

class RenderSystem;

class OpenGL_Renderer : public IRenderer {
public:
    OpenGL_Renderer(RenderSystem& system, const Window* window);

    auto present() -> void override;
    auto clear_background() -> void override;
    auto render(const Camera& camera) -> void override;

    auto set_clear_color(const RGBAColor& color) -> void override;
    auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void override;

    auto take_screenshot(const char* filename) -> Image override;

private:
    auto render_mesh(
        const Mesh*        vertex_data,
        const GPUMeshData* gpu_data,
        OGLW_VertexArray*  vao
    ) -> void;

public:
    OpenGL_Context m_context;
    RenderSystem*  m_system = nullptr;

    struct RenderTarget {
        Object                m_fb;
        opengl::Texture*      m_texture;
        opengl::Renderbuffer* m_renderbuffer;
        opengl::Framebuffer*  m_framebuffer;
        opengl::Buffer*       m_vertex_buffer;
        ShaderHandle*         m_shader;
        OpenGL_Context*       m_context;

        auto init(OpenGL_Context* context, RenderSystem* system) -> void;
        auto render(RenderSystem* system) -> void;
    } m_render_target;
};

static_assert(is_renderer<OpenGL_Renderer>);

} // namespace JadeFrame