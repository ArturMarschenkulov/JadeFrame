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
#include "stb/stb_image_write.h"
#include <chrono>
#include <future>
#include <thread>

namespace JadeFrame {
namespace gl {}

auto OpenGL_Renderer::set_clear_color(const RGBAColor& color) -> void { m_context.m_state.set_clear_color(color); }

auto OpenGL_Renderer::clear_background() -> void {
    GLbitfield bitfield = m_context.m_state.clear_bitfield;
    glClear(bitfield);
}
auto OpenGL_Renderer::set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void {
    m_context.m_state.set_viewport(x, y, width, height);
}
#define JF_FB 1
OpenGL_Renderer::OpenGL_Renderer(RenderSystem& system, const IWindow* window)
    : m_context(window) {
    m_system = &system;
    {
        // setup_framebuffer(m_framebuffer, m_framebuffer_texture, m_framebuffer_renderbuffer);

        fb.m_framebuffer = opengl::Framebuffer(m_context);
        fb.m_framebuffer.bind();

        const v2u32 size = m_context.m_state.viewport[1];
        fb.m_framebuffer_texture = m_context.create_texture();
        fb.m_framebuffer_texture->bind(0);

        fb.m_framebuffer_texture->set_image(0, GL_RGB, size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        fb.m_framebuffer_texture->set_parameters(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        fb.m_framebuffer_texture->set_parameters(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        fb.m_framebuffer_texture->set_parameters(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        fb.m_framebuffer_texture->set_parameters(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        fb.m_framebuffer_renderbuffer.bind();
        fb.m_framebuffer_renderbuffer.store(GL_DEPTH24_STENCIL8, size.x, size.y);

        fb.m_framebuffer.attach_texture(opengl::ATTACHMENT::COLOR, 0, *fb.m_framebuffer_texture);
        fb.m_framebuffer.attach_renderbuffer(opengl::ATTACHMENT::DEPTH_STENCIL, 0, fb.m_framebuffer_renderbuffer);

        fb.m_framebuffer.unbind();

        const GLenum res = fb.m_framebuffer.check_status();
        if (res != GL_FRAMEBUFFER_COMPLETE) {
            Logger::err("OpenGL_Renderer::OpenGL_Renderer: Framebuffer is not complete");
            assert(false);
        }
    }



    VertexData::Desc vdf_desc;
    vdf_desc.has_normals = false;
    VertexData vertex_data = VertexData::make_rectangle({-1.0f, -1.0f, 0.0f}, {2.0f, 2.0f, 0.0f}, vdf_desc);

    VertexFormat layout = {
        {           "v_position", SHADER_TYPE::V_3_F32},
        {"v_texture_coordinates", SHADER_TYPE::V_2_F32}
    };
    fb.m_framebuffer_rect = new opengl::GPUMeshData(m_context, vertex_data, layout);

    ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("framebuffer_test");
    shader_handle_desc.vertex_format = layout;
#if JF_FB
    fb.m_shader = m_system->register_shader(shader_handle_desc);
#endif


    {
        const GLuint binding_point_0 = 0;
        const GLuint binding_point_1 = 1;

        auto cam_ubo = m_context.create_buffer(opengl::Buffer::TYPE::UNIFORM, nullptr, sizeof(Matrix4x4));
        cam_ubo->bind_base(binding_point_0);
        m_uniform_buffers.push_back(cam_ubo);

        auto transform_ubo = m_context.create_buffer(opengl::Buffer::TYPE::UNIFORM, nullptr, sizeof(Matrix4x4));
        transform_ubo->bind_base(binding_point_1);
        m_uniform_buffers.push_back(transform_ubo);
    }
}

auto OpenGL_Renderer::present() -> void { m_context.swap_buffers(); }
auto OpenGL_Renderer::submit(const Object& obj) -> void {

    const OpenGL_RenderCommand command = {
        .transform = &obj.m_transform,
        .vertex_data = obj.m_vertex_data,
        .material_handle = obj.m_material_handle,
        .m_GPU_mesh_data = &obj.m_GPU_mesh_data,
        .m_GPU_mesh_data_id = obj.m_vertex_data_id,
    };
    m_render_commands.push_back(command);
}


auto OpenGL_Renderer::render(const Matrix4x4& view_projection) -> void {


#if JF_FB
    fb.m_framebuffer.bind();
#endif

    this->clear_background();

    m_uniform_buffers[0]->update({view_projection});

    for (size_t i = 0; i < m_render_commands.size(); ++i) {
        const OpenGL_RenderCommand& command = m_render_commands[i];
        const MaterialHandle&       mh = command.material_handle;

        auto& sh = m_system->m_registered_shaders[mh.m_shader_id];
        auto& mm = m_registered_meshes[command.m_GPU_mesh_data_id];

        const opengl::Shader*      p_shader = static_cast<opengl::Shader*>(sh.m_handle);
        const VertexData*          p_mesh = command.vertex_data;
        const opengl::GPUMeshData* p_vertex_array = &mm;

        p_shader->bind();
        if (mh.m_texture_id != 0) {
            auto&            th = m_system->m_registered_textures[mh.m_texture_id];
            opengl::Texture& texture = *static_cast<opengl::Texture*>(th.m_handle);
            texture.bind(0);
        }

        const Matrix4x4& transform = *command.transform;
        m_uniform_buffers[1]->update({transform});

        this->render_mesh(p_vertex_array, p_mesh);
    }
#if JF_FB
    fb.m_framebuffer.unbind();
    // GL_State old_state = m_context.m_state;
    m_context.m_state.set_depth_test(false);
    {

        // static_cast<opengl::Shader*>(fb.m_shader_handle_fb->m_handle)->bind();
        ShaderHandle&   sh_ = m_system->m_registered_shaders[fb.m_shader];
        opengl::Shader* sh = static_cast<opengl::Shader*>(sh_.m_handle);
        sh->bind();
        fb.m_framebuffer_texture->bind(0);
        fb.m_framebuffer_rect->m_vertex_array.bind();

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    m_context.m_state.set_depth_test(true);
#endif
#undef JF_FB
    m_render_commands.clear();
}

auto OpenGL_Renderer::render_mesh(const opengl::GPUMeshData* vertex_array, const VertexData* vertex_data) const
    -> void {
    vertex_array->bind();

    if (vertex_data->m_indices.size() > 0) {
        glDrawElements(
            static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES), // mode
            vertex_data->m_indices.size(),                  // count
            GL_UNSIGNED_INT,                                // type
            nullptr                                         // indices
        );
    } else {
        glDrawArrays(
            static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES), // mode
            0,                                              // first
            vertex_data->m_positions.size()                 // count
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
    if (!data) {
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