#include "pch.h"

#include "opengl_renderer.h"
#include "../shader_loader.h"
// #include "JadeFrame/base_app.h"
// #include "graphics/opengl/opengl_renderer.h"
// #include "graphics/vulkan/vulkan_renderer.h"
#include "JadeFrame/graphics/mesh.h"

#ifdef _WIN32
    #include "Windows.h"
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#undef __OPTIMIZE__
JF_PRAGMA_NO_WARNINGS_PUSH
#include "stb/stb_image_write.h"
JF_PRAGMA_NO_WARNINGS_POP
#include <thread>

namespace JadeFrame {
namespace gl {}

auto OpenGL_Renderer::set_clear_color(const RGBAColor& color) -> void {
    m_context.m_state.set_clear_color(color);
}

auto OpenGL_Renderer::clear_background() -> void {
    GLbitfield bitfield = m_context.m_state.clear_bitfield;
    glClear(bitfield);
}

auto OpenGL_Renderer::set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void {
    m_context.m_state.set_viewport(x, y, width, height);
}

static auto framebuffer_res_to_str(GLenum e) -> const char* {
#define foo(name)                                                                        \
    case name: return &#name[sizeof("GL_FRAMEBUFFER_") - 1]
    switch (e) {
        foo(GL_FRAMEBUFFER_UNDEFINED);
        foo(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
        foo(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
        foo(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
        foo(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
        foo(GL_FRAMEBUFFER_UNSUPPORTED);
        foo(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
        foo(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
        foo(GL_FRAMEBUFFER_COMPLETE);
        default: return "UNKNOWN";
    }
#undef foo
}

OpenGL_Renderer::OpenGL_Renderer(RenderSystem& system, const Window* window)
    : m_context(window)
    , m_system(&system) {

#if JF_OPENGL_FB
    m_render_target.init(&m_context, m_system);
#endif
}

auto OpenGL_Renderer::present() -> void { m_context.m_swapchain_context.swap_buffers(); }

auto OpenGL_Renderer::render(const mat4x4& view_projection) -> void {

#if JF_OPENGL_FB
    m_context.bind_framebuffer(*m_render_target.m_framebuffer);
#endif

    this->clear_background();

    // NOTE: At the time of writing this is mainly compatible with
    // `get_shader_spirv_test_1` or rather on any renderer where the camera uniform is
    // at binding point 0 and the transform uniform is at binding point 1.
    auto& render_commands = m_system->m_render_commands;

    for (size_t i = 0; i < render_commands.size(); ++i) {
        const RenderCommand&  cmd = render_commands[i];
        const MaterialHandle& mh = *cmd.material;

        auto* shader = static_cast<opengl::Shader*>(mh.m_shader->m_handle);
        m_context.bind_shader(*shader);

        // NOTE: As of right now this is not optimal, as it only needs to be updated once
        // outside the loop. But because of how the code is arranged one has to update it
        // every iteration of the loop. Late on one HAS TO fix this.

        // ub_cam
        shader->write_ub(0, &view_projection, sizeof(view_projection), 0);

        // ub_tran
        shader->write_ub(1, &cmd.transform, sizeof(cmd.transform), 0);

        if (mh.m_texture != nullptr) {
            auto* texture = static_cast<opengl::Texture*>(mh.m_texture->m_handle);

            u32 texture_unit = 0;
            m_context.bind_texture_to_unit(*texture, texture_unit);
        }

        OpenGL_Renderer::render_mesh(
            cmd.vertex_data, cmd.m_mesh, &shader->m_vertex_array
        );
    }
#if JF_OPENGL_FB
    m_context.unbind_framebuffer();
    m_context.m_state.set_depth_test(false);
    m_render_target.render(m_system);
    m_context.m_state.set_depth_test(true);
#endif
#undef JF_OPENGL_FB
    render_commands.clear();
}

template<typename T>
static auto to_opengl_type() -> GLenum {
    if constexpr (std::is_same_v<T, f32>) {
        return GL_FLOAT;
    } else if constexpr (std::is_same_v<T, i32>) {
        return GL_INT;
    } else if constexpr (std::is_same_v<T, u32>) {
        return GL_UNSIGNED_INT;
    } else if constexpr (std::is_same_v<T, u8>) {
        return GL_UNSIGNED_BYTE;
    } else {
        // static_assert(false, "Unsupported type");
        return 0;
    }
}

auto OpenGL_Renderer::render_mesh(
    const VertexData*  vertex_data,
    const GPUMeshData* gpu_data,
    OGLW_VertexArray*  vao
) -> void {
    // TODO: Considering we are replicating Vulkan's way of doing things, the primitive
    // type should be defined in the pipeline or shader, and here it should be simply
    // queried.

    m_context.bind_vertex_array(*vao);
    vao->bind_buffer(*static_cast<opengl::Buffer*>(gpu_data->m_vertex_buffer->m_handle));

    auto prim_type = static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES);
    if (!vertex_data->m_indices.empty()) {
        auto num_indices = static_cast<GLsizei>(vertex_data->m_indices.size());
        // auto gl_type = to_opengl_type<u32>();
        auto gl_type = GL_UNSIGNED_INT;
        glDrawElements(prim_type, num_indices, gl_type, nullptr);
    } else {
        auto num_vertices = static_cast<GLsizei>(vertex_data->m_positions.size());
        glDrawArrays(prim_type, 0, num_vertices);
    }
}

auto OpenGL_Renderer::take_screenshot(const char* filename) -> Image {
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    i32 x = vp[0];
    i32 y = vp[1];
    i32 width = vp[2];
    i32 height = vp[3];

    u8* data = static_cast<u8*>(malloc((size_t)(width * height * 3)));
    if (data == nullptr) {
        Logger::log("data failed");
        Image image = {};
        image.data = nullptr;
        return image;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

    Image image;
    image.data = data;
    image.width = width;
    image.height = height;
    image.num_components = 3;
    return image;

    // auto c = [](const char* filename, i32 width, i32 height, u8* data) {
    //     stbi_write_png(filename, width, height, 3, data, 0);
    //     free(data);
    // };
    // std::thread t(c, filename, width, height, data);
    // t.detach();
}

auto OpenGL_Renderer::RenderTarget::init(OpenGL_Context* context, RenderSystem* system)
    -> void {
    m_context = context;
    {
        // TODO: the use of `GL_RGB8`, `GL_RGB` and `GL_UNSIGNED_BYTE` is hardcoded. Find
        // a way to make it more flexible.
        const v2u32 size = context->m_state.viewport[1];

        m_texture = context->create_texture();
        m_texture->set_image(0, GL_RGB8, size, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        m_texture->set_parameters(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        m_texture->set_parameters(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        m_texture->set_parameters(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        m_texture->set_parameters(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        m_renderbuffer = context->create_renderbuffer();
        m_renderbuffer->store(GL_DEPTH24_STENCIL8, size.x, size.y);

        m_framebuffer = context->create_framebuffer();
        m_framebuffer->attach(opengl::ATTACHMENT::COLOR, 0, *m_texture);
        m_framebuffer->attach(opengl::ATTACHMENT::DEPTH_STENCIL, 0, *m_renderbuffer);

        const GLenum res = m_framebuffer->check_status();
        if (res != GL_FRAMEBUFFER_COMPLETE) {
            Logger::err(
                "OpenGL_Renderer::OpenGL_Renderer: Framebuffer is not complete, status: "
                "{}",
                framebuffer_res_to_str(res)
            );
            // assert(false);
        }
    }

    VertexData::Desc vdf_desc;
    vdf_desc.has_normals = false;
    VertexData vertex_data =
        VertexData::rectangle({-1.0F, -1.0F, 0.0F}, {2.0F, 2.0F, 0.0F}, vdf_desc);
    auto data = convert_into_data(vertex_data, true);
    u32  size = static_cast<u32>(data.size() * sizeof(f32));
    m_vertex_buffer =
        context->create_buffer(opengl::Buffer::TYPE::VERTEX, data.data(), size);

    ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("framebuffer_test");
    m_shader = system->register_shader(shader_handle_desc);
}

auto OpenGL_Renderer::RenderTarget::render(RenderSystem* system) -> void {

    ShaderHandle& sh_ = *m_shader;
    auto*         sh = static_cast<opengl::Shader*>(sh_.m_handle);
    m_context->bind_shader(*sh);
    m_context->bind_texture_to_unit(*m_texture, 0);
    m_context->bind_vertex_array(sh->m_vertex_array);
    sh->m_vertex_array.bind_buffer(*m_vertex_buffer);

    const GLsizei num_vertices = 6;
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
}
} // namespace JadeFrame