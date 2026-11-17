#pragma once

#include <functional>

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

enum class PRIMITIVE_TYPE : u8 {
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
    OpenGL_Renderer(RenderSystem& system, Window* window);

    [[nodiscard]] auto initialize() -> Result<void, InitError> override;
    auto create_buffer(GPUBuffer::TYPE type, const void* data, size_t size)
        -> void* override;
    auto destroy_buffer(void* handle) -> void override;
    auto create_texture(const Image& image) -> NativeHandle override;
    auto create_shader(const ShadingCode& code) -> NativeHandle override;
    auto try_create_shader(const ShadingCode& code)
        -> Result<NativeHandle, RendererError> override;
    auto create_material(const ShaderHandle& shader, const TextureHandle* texture)
        -> NativeHandle override;
    auto set_shader_uniform(
        void*             shader,
        const std::string& name,
        const void*        data,
        size_t             size
    ) -> Result<void, RendererError> override;

    auto present() -> void override;
    auto wait_until_idle() -> void override;
    auto clear_background() -> void override;
    auto render(const Camera& camera) -> void override;

    auto set_clear_color(const RGBAColor& color) -> void override;
    auto set_viewport(u32 x, u32 y, u32 width, u32 height) -> void override;

    auto take_screenshot(const char* filename) -> Result<Image, RendererError> override;

private:
    friend class RenderSystem;

    auto render_mesh(
        const GPUMeshData* gpu_data,
        OGLW_VertexArray*  vao
    ) -> void;

public:
    // Declared before everything that holds OpenGL objects, so that it is destroyed
    // last.
    opengl::Context m_context;
    RenderSystem*   m_system = nullptr;
    Window*         m_window = nullptr;
    // True from the end of `render()` until `present()`, see `take_screenshot`.
    bool            m_frame_rendered = false;

    struct RenderTarget {
        Object                m_fb;
        opengl::Texture*      m_texture = nullptr;
        opengl::Sampler*      m_sampler = nullptr;
        opengl::Renderbuffer* m_renderbuffer = nullptr;
        opengl::Framebuffer*  m_framebuffer = nullptr;
        opengl::Buffer*       m_vertex_buffer = nullptr;
        opengl::Buffer*       m_index_buffer = nullptr;
        Mesh                  m_mesh;
        // Owned by the render target. It is an internal resource of the renderer and
        // therefore not registered with the `RenderSystem`.
        NativeHandle          m_shader = {nullptr, noop_native_handle_deleter};
        opengl::Context*      m_context = nullptr;
        // Size of the attachments. Kept equal to the size of the window's framebuffer.
        v2u32                 m_size = {};

        auto resize(v2u32 size) -> void;
        /// Creates the offscreen framebuffer and what is needed to copy it onto the
        /// window. `shader` is the compiled "framebuffer_test" shader. Everything
        /// created here is owned by `context` or by this object, so a failure needs
        /// no cleanup beyond their destruction.
        [[nodiscard]] auto init(opengl::Context* context, NativeHandle shader)
            -> Result<void, InitError>;
        auto render(RenderSystem* system) -> void;
    } m_render_target;
};

static_assert(is_renderer<OpenGL_Renderer>);

} // namespace JadeFrame
