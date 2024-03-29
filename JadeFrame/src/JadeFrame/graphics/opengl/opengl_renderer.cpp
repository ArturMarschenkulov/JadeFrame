#include "pch.h"

#include "opengl_renderer.h"
// #include "JadeFrame/base_app.h"
// #include "graphics/opengl/opengl_renderer.h"
// #include "graphics/vulkan/vulkan_renderer.h"

#ifdef _WIN32
    #include "Windows.h"
#endif

#include <cassert>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#undef __OPTIMIZE__
JF_PRAGMA_NO_WARNINGS_PUSH
#include "stb/stb_image_write.h"
JF_PRAGMA_NO_WARNINGS_POP
#include <chrono>
#include <future>
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
    switch (e) {
        case GL_FRAMEBUFFER_UNDEFINED: return "GL_FRAMEBUFFER_UNDEFINED";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        case GL_FRAMEBUFFER_UNSUPPORTED: return "GL_FRAMEBUFFER_UNSUPPORTED";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
        case GL_FRAMEBUFFER_COMPLETE: return "GL_FRAMEBUFFER_COMPLETE";
        default: return "UNKNOWN";
    }
}

OpenGL_Renderer::OpenGL_Renderer(RenderSystem& system, const IWindow* window)
    : m_context(window) {
    m_system = &system;

    // TODO: The whole fb thing is now a mess!!!! This is because
    // `m_system->register_shader(shader_handle_desc);` requires an already initialized
    // `OpenGL_Renderer`. Thus for now `JF_OPENGL_FB` should be defined to 0, because
    // otherwise it will always crash!!!!

    // TODO: Now it somehow works again, however I do not think I have changed anything to
    // actually solve this. This might be some undefined behavior. Thus one should watch
#if JF_OPENGL_FB
    {
        fb.m_framebuffer = m_context.create_framebuffer();

        const v2u32 size = m_context.m_state.viewport[1];

        fb.m_texture = m_context.create_texture();
        fb.m_texture->bind(0);
        fb.m_texture->set_image(0, GL_RGB, size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        fb.m_texture->set_parameters(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        fb.m_texture->set_parameters(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        fb.m_texture->set_parameters(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        fb.m_texture->set_parameters(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        fb.m_renderbuffer = m_context.create_renderbuffer();
        fb.m_renderbuffer->store(GL_DEPTH24_STENCIL8, size.x, size.y);

        fb.m_framebuffer->attach_texture(opengl::ATTACHMENT::COLOR, 0, *fb.m_texture);
        fb.m_framebuffer->attach_renderbuffer(
            opengl::ATTACHMENT::DEPTH_STENCIL, 0, *fb.m_renderbuffer
        );

        const GLenum res = fb.m_framebuffer->check_status();
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
    VertexFormat layout = {
        {           "v_position", SHADER_TYPE::V_3_F32},
        {"v_texture_coordinates", SHADER_TYPE::V_2_F32}
    };
    fb.m_framebuffer_rect = new opengl::GPUMeshData(m_context, vertex_data, layout);

    ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("framebuffer_test");
    shader_handle_desc.vertex_format = layout;

    fb.m_shader = m_system->register_shader(shader_handle_desc);
#endif
}

auto OpenGL_Renderer::present() -> void { m_context.swap_buffers(); }

auto OpenGL_Renderer::submit(const Object& obj) -> void {

    const OpenGL_RenderCommand command = {
        .transform = &obj.m_transform,
        .vertex_data = obj.m_vertex_data,
        .material_handle = obj.m_material_handle,
        .m_GPU_mesh_data_id = obj.m_vertex_data_id,
    };
    m_render_commands.push_back(command);
}

auto OpenGL_Renderer::render(const Matrix4x4& view_projection) -> void {

#if JF_OPENGL_FB
    fb.m_framebuffer->bind();
#endif

    this->clear_background();

    // NOTE: At the time of writing this is mainly compatible with
    // `get_shader_spirv_test_1` or rather on any renderer where the camera uniform is
    // at binding point 0 and the transform uniform is at binding point 1.

    for (size_t i = 0; i < m_render_commands.size(); ++i) {
        const OpenGL_RenderCommand& command = m_render_commands[i];
        const MaterialHandle&       mh = command.material_handle;

        auto& sh = m_system->m_registered_shaders[mh.m_shader_id];
        auto& mm = m_registered_meshes[command.m_GPU_mesh_data_id];

        const opengl::Shader*      p_shader = static_cast<opengl::Shader*>(sh.m_handle);
        const VertexData*          p_mesh = command.vertex_data;
        const opengl::GPUMeshData* p_vertex_array = &mm;

        // NOTE: As of right now this is not optimal, as it only needs to be updated once
        // outside the loop. But because of how the code is arranged one has to update it
        // every iteration of the loop. Late on one HAS TO fix this.
        p_shader->m_uniform_buffers[0]->write({view_projection});

        p_shader->bind();
        if (mh.m_texture_id != 0) {
            auto&            th = m_system->m_registered_textures[mh.m_texture_id];
            opengl::Texture& texture = *static_cast<opengl::Texture*>(th.m_handle);
            texture.bind(0);
        }

        const Matrix4x4& transform = *command.transform;
        p_shader->m_uniform_buffers[1]->write({transform});

        OpenGL_Renderer::render_mesh(p_vertex_array, p_mesh);
    }
#if JF_OPENGL_FB
    fb.m_framebuffer->unbind();
    // GL_State old_state = m_context.m_state;
    m_context.m_state.set_depth_test(false);
    {

        // static_cast<opengl::Shader*>(fb.m_shader_handle_fb->m_handle)->bind();
        ShaderHandle& sh_ = m_system->m_registered_shaders[fb.m_shader];
        auto*         sh = static_cast<opengl::Shader*>(sh_.m_handle);
        sh->bind();
        fb.m_texture->bind(0);
        fb.m_framebuffer_rect->m_vertex_array.bind_buffer(
            *fb.m_framebuffer_rect->m_vertex_buffer
        );
        fb.m_framebuffer_rect->m_vertex_array.bind();

        const GLsizei num_vertices = 6;
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }
    m_context.m_state.set_depth_test(true);
#endif
#undef JF_OPENGL_FB
    m_render_commands.clear();
}

auto OpenGL_Renderer::render_mesh(
    const opengl::GPUMeshData* buffer_data,
    const VertexData*          vertex_data
) -> void {
    buffer_data->m_vertex_array.bind_buffer(*buffer_data->m_vertex_buffer);
    buffer_data->m_vertex_array.bind();

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

    u8* data = (u8*)malloc((size_t)(width * height * 3));
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

} // namespace JadeFrame