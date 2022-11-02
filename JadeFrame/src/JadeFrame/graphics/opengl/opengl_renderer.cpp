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

    //__debugbreak();
}

// static auto
// setup_framebuffer(OGLW_Framebuffer& buffer, OGLW_Texture<GL_TEXTURE_2D>& texture, OGLW_Renderbuffer& renderbuffer)
//     -> void {
//     buffer.bind();

//     const v2i32 size; // = m_context.m_cache.viewport[1];
//     texture.bind(0);

//     texture.set_texture_image_2D(0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
//     texture.set_texture_parameters(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     texture.set_texture_parameters(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     texture.set_texture_parameters(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     texture.set_texture_parameters(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     buffer.attach_texture(texture);

//     renderbuffer.bind();
//     renderbuffer.store(GL_DEPTH24_STENCIL8, size.x, size.y);
//     buffer.attach_renderbuffer(renderbuffer);

//     buffer.unbind();

//     const GLenum res = buffer.check_status();
//     if (res != GL_FRAMEBUFFER_COMPLETE) assert(false);
// }
OpenGL_Renderer::OpenGL_Renderer(const IWindow* window)
    : m_context(window) {
    {
        // setup_framebuffer(m_framebuffer, m_framebuffer_texture, m_framebuffer_renderbuffer);

        fb.m_framebuffer.bind();

        const v2u32 size = m_context.m_state.viewport[1];
        fb.m_framebuffer_texture.bind(0);

        fb.m_framebuffer_texture.set_texture_image(0, GL_RGB, size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        fb.m_framebuffer_texture.set_texture_parameters(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        fb.m_framebuffer_texture.set_texture_parameters(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        fb.m_framebuffer_texture.set_texture_parameters(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        fb.m_framebuffer_texture.set_texture_parameters(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        fb.m_framebuffer.attach_texture(fb.m_framebuffer_texture);

        fb.m_framebuffer_renderbuffer.bind();
        fb.m_framebuffer_renderbuffer.store(GL_DEPTH24_STENCIL8, size.x, size.y);
        fb.m_framebuffer.attach_renderbuffer(fb.m_framebuffer_renderbuffer);

        fb.m_framebuffer.unbind();

        const GLenum res = fb.m_framebuffer.check_status();
        if (res != GL_FRAMEBUFFER_COMPLETE) assert(false);
    }



    VertexDataFactory::Desc vdf_desc;
    vdf_desc.has_normals = false;
    VertexData vertex_data = VertexDataFactory::make_rectangle({-1.0f, -1.0f, 0.0f}, {2.0f, 2.0f, 0.0f}, vdf_desc);

    VertexFormat layout = {
        {           "v_position", SHADER_TYPE::FLOAT_3},
        {"v_texture_coordinates", SHADER_TYPE::FLOAT_2}
    };
    fb.m_framebuffer_rect = new opengl::GPUMeshData(vertex_data, layout);

    ShaderHandle::DESC shader_handle_desc;
    shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("framebuffer_test");
    shader_handle_desc.vertex_format = layout;
    fb.m_shader_handle_fb = new ShaderHandle(shader_handle_desc);
    fb.m_shader_handle_fb->m_api = GRAPHICS_API::OPENGL;
    fb.m_shader_handle_fb->init();
}

auto OpenGL_Renderer::present() -> void { m_context.swap_buffers(); }
auto OpenGL_Renderer::submit(const Object& obj) -> void {

    if (obj.m_GPU_mesh_data.m_is_initialized == false) {
        VertexFormat vertex_format;
        // In case there is no buffer layout provided use a default one
        if (obj.m_vertex_format.m_attributes.size() == 0) {
            const VertexFormat vf = {
                {     "v_position", SHADER_TYPE::FLOAT_3},
                {        "v_color", SHADER_TYPE::FLOAT_4},
                {"v_texture_coord", SHADER_TYPE::FLOAT_2},
                {       "v_normal", SHADER_TYPE::FLOAT_3},
            };
            vertex_format = vf;
        } else {
            vertex_format = obj.m_vertex_format;
        }

        obj.m_GPU_mesh_data.m_handle = new opengl::GPUMeshData(*obj.m_vertex_data, vertex_format);
        obj.m_GPU_mesh_data.m_is_initialized = true;
    }
    MaterialHandle* mh = obj.m_material_handle;
    ShaderHandle*   sh = mh->m_shader_handle;
    TextureHandle*  th = mh->m_texture_handle;
    if (mh->m_is_initialized == false) {
        // obj.m_material_handle->init();
        sh->m_api = GRAPHICS_API::OPENGL;
        sh->init();
        // obj.m_material_handle->m_shader_handle->m_handle = new OpenGL_Shader();

        if (th != nullptr) {
            th->m_api = GRAPHICS_API::OPENGL;
            th->init();
        }
        obj.m_material_handle->m_is_initialized = true;
    }


    const OpenGL_RenderCommand command = {
        .transform = &obj.m_transform,
        .vertex_data = obj.m_vertex_data,
        .material_handle = obj.m_material_handle,
        .m_GPU_mesh_data = &obj.m_GPU_mesh_data,
    };
    m_render_commands.push_back(command);
}


auto OpenGL_Renderer::render(const Matrix4x4& view_projection) -> void {

#define JF_FB 1
#if JF_FB
    fb.m_framebuffer.bind();
#endif

    this->clear_background();

    m_context.m_uniform_buffers[0].bind();
    m_context.m_uniform_buffers[0].send({view_projection});
    m_context.m_uniform_buffers[0].unbind();

    for (size_t i = 0; i < m_render_commands.size(); ++i) {
        const OpenGL_RenderCommand& command = m_render_commands[i];
        const MaterialHandle&       mh = *command.material_handle;

        const opengl::Shader*      p_shader = static_cast<opengl::Shader*>(mh.m_shader_handle->m_handle);
        const VertexData*         p_mesh = command.vertex_data;
        const opengl::GPUMeshData* p_vertex_array = static_cast<opengl::GPUMeshData*>(command.m_GPU_mesh_data->m_handle);

        p_shader->bind();
        if (mh.m_texture_handle != nullptr) {
            opengl::Texture& texture = *static_cast<opengl::Texture*>(mh.m_texture_handle->m_handle);
            texture.bind();
        }

        this->render_mesh(p_vertex_array, p_mesh);

        const Matrix4x4& transform = *command.transform;
        m_context.m_uniform_buffers[1].bind();
        m_context.m_uniform_buffers[1].send({transform});
        m_context.m_uniform_buffers[1].unbind();
    }
#if JF_FB
    fb.m_framebuffer.unbind();
    // GL_State old_state = m_context.m_state;
    m_context.m_state.set_depth_test(false);
    {

        static_cast<OpenGL_Shader*>(fb.m_shader_handle_fb->m_handle)->bind();
        fb.m_framebuffer_texture.bind(0);
        fb.m_framebuffer_rect->m_vertex_array.bind();

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    m_context.m_state.set_depth_test(true);
#endif
#undef JF_FB
    m_render_commands.clear();
}

auto OpenGL_Renderer::render_mesh(const opengl::GPUMeshData* vertex_array, const VertexData* vertex_data) const -> void {
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

} // namespace JadeFrame