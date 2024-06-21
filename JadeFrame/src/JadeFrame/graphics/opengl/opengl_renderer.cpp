#include "pch.h"

#include "opengl_renderer.h"
#include "../shader_loader.h"
// #include "JadeFrame/base_app.h"
// #include "graphics/opengl/opengl_renderer.h"
// #include "graphics/vulkan/vulkan_renderer.h"

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
    fb.init(&m_context, m_system);
#endif
}

auto OpenGL_Renderer::present() -> void { m_context.swap_buffers(); }

auto OpenGL_Renderer::render(const Matrix4x4& view_projection) -> void {

#if JF_OPENGL_FB
    fb.m_framebuffer->bind();
#endif

    this->clear_background();

    // NOTE: At the time of writing this is mainly compatible with
    // `get_shader_spirv_test_1` or rather on any renderer where the camera uniform is
    // at binding point 0 and the transform uniform is at binding point 1.
    auto& m_render_commands = m_system->m_render_commands;

    for (size_t i = 0; i < m_render_commands.size(); ++i) {
        const RenderCommand&  cmd = m_render_commands[i];
        const MaterialHandle& mh = *cmd.material;

        auto&                 sh = *mh.m_shader;
        const opengl::Shader* shader = static_cast<opengl::Shader*>(sh.m_handle);

        // NOTE: As of right now this is not optimal, as it only needs to be updated once
        // outside the loop. But because of how the code is arranged one has to update it
        // every iteration of the loop. Late on one HAS TO fix this.
        auto* ub_cam = shader->m_uniform_buffers[0];
        ub_cam->write({view_projection});

        auto* ub_tran = shader->m_uniform_buffers[1];
        ub_tran->write({*cmd.transform});

        shader->bind();
        if (mh.m_texture != nullptr) {
            auto*            th = mh.m_texture;
            opengl::Texture& texture = *static_cast<opengl::Texture*>(th->m_handle);

            u32 texture_unit = 0;
            texture.bind(texture_unit);
        }

        GPUMeshData& gpu_data = *cmd.m_mesh;
        shader->m_vertex_array.bind_buffer(
            *static_cast<opengl::Buffer*>(gpu_data.m_vertex_buffer->m_handle)
        );
        shader->m_vertex_array.bind();
        OpenGL_Renderer::render_mesh(cmd.vertex_data);
    }
#if JF_OPENGL_FB
    fb.m_framebuffer->unbind();
    // GL_State old_state = m_context.m_state;
    m_context.m_state.set_depth_test(false);
    fb.render(m_system);
    m_context.m_state.set_depth_test(true);
#endif
#undef JF_OPENGL_FB
    m_render_commands.clear();
}

auto OpenGL_Renderer::render_mesh(const VertexData* vertex_data) -> void {

    if (!vertex_data->m_indices.empty()) {
        glDrawElements(
            static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES),      // mode
            static_cast<GLsizei>(vertex_data->m_indices.size()), // count
            GL_UNSIGNED_INT,                                     // type
            nullptr                                              // indices
        );
    } else {
        glDrawArrays(
            static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES),       // mode
            0,                                                    // first
            static_cast<GLsizei>(vertex_data->m_positions.size()) // count
        );
    }
}

auto OpenGL_Renderer::take_screenshot(const char* filename) -> void {
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    i32 x = vp[0];
    i32 y = vp[1];
    i32 width = vp[2];
    i32 height = vp[3];

    u8* data = static_cast<u8*>(malloc((size_t)(width * height * 3)));
    if (data == nullptr) {
        Logger::log("data failed");
        return;
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

    auto c = [](const char* filename, i32 width, i32 height, u8* data) {
        stbi_write_png(filename, width, height, 3, data, 0);
        free(data);
    };
    std::thread t(c, filename, width, height, data);
    t.detach();
}

auto OpenGL_Renderer::FB::init(OpenGL_Context* context, RenderSystem* system) -> void {
    { // TODO: The whole fb thing is now a mess!!!! This is because
        // `m_system->register_shader(shader_handle_desc);` requires an already
        // initialized `OpenGL_Renderer`. Thus for now `JF_OPENGL_FB` should be defined to
        // 0, because otherwise it will always crash!!!!

        // TODO: Now it somehow works again, however I do not think I have changed
        // anything to actually solve this. This might be some undefined behavior. Thus
        // one should watch
        m_framebuffer = context->create_framebuffer();

        const v2u32 size = context->m_state.viewport[1];

        m_texture = context->create_texture();
        m_texture->bind(0);
        m_texture->set_image(0, GL_RGB, size, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        m_texture->set_parameters(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        m_texture->set_parameters(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        m_texture->set_parameters(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        m_texture->set_parameters(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        m_renderbuffer = context->create_renderbuffer();
        m_renderbuffer->store(GL_DEPTH24_STENCIL8, size.x, size.y);

        m_framebuffer->attach_texture(opengl::ATTACHMENT::COLOR, 0, *m_texture);
        m_framebuffer->attach_renderbuffer(
            opengl::ATTACHMENT::DEPTH_STENCIL, 0, *m_renderbuffer
        );

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
        VertexData::make_rectangle({-1.0F, -1.0F, 0.0F}, {2.0F, 2.0F, 0.0F}, vdf_desc);
    VertexFormat layout = VertexFormat({
        {           "v_position", SHADER_TYPE::V_3_F32},
        {"v_texture_coordinates", SHADER_TYPE::V_2_F32}
    });
    m_framebuffer_rect = new opengl::GPUMeshData(*context, vertex_data);

    ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("framebuffer_test");
    shader_handle_desc.vertex_format = layout;

    m_shader = system->register_shader(shader_handle_desc);
}

auto OpenGL_Renderer::FB::render(RenderSystem* system) -> void {

    // static_cast<opengl::Shader*>(fb.m_shader_handle_fb->m_handle)->bind();
    ShaderHandle& sh_ = *m_shader;
    auto*         sh = static_cast<opengl::Shader*>(sh_.m_handle);
    sh->bind();
    m_texture->bind(0);
    auto&                 shh = *m_shader;
    const opengl::Shader* p_shader = static_cast<opengl::Shader*>(shh.m_handle);
    p_shader->m_vertex_array.bind_buffer(*m_framebuffer_rect->m_vertex_buffer);
    p_shader->m_vertex_array.bind();

    const GLsizei num_vertices = 6;
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
}
} // namespace JadeFrame