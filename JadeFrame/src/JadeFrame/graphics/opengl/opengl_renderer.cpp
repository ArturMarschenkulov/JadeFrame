

#include <cassert>
#ifdef _WIN32
    #include "Windows.h"
#endif

#include "JadeFrame/macros.h"

#include "opengl_renderer.h"
#include "JadeFrame/utils/logger.h"
#include "../graphics_language.h"
#include "../shader_loader.h"
#include "JadeFrame/graphics/mesh.h"

#include <algorithm>
#include <limits>

#define JF_OPENGL_FB 1

namespace JadeFrame {
namespace gl {}

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

static auto to_opengl(GPUBuffer::TYPE type) -> opengl::Buffer::TYPE {
    switch (type) {
        case GPUBuffer::TYPE::VERTEX: return opengl::Buffer::TYPE::VERTEX;
        case GPUBuffer::TYPE::INDEX: return opengl::Buffer::TYPE::INDEX;
        case GPUBuffer::TYPE::UNIFORM: return opengl::Buffer::TYPE::UNIFORM;
        case GPUBuffer::TYPE::STAGING: return opengl::Buffer::TYPE::STAGING;
        default: assert(false); return opengl::Buffer::TYPE::VERTEX;
    }
}

static auto delete_opengl_shader(void* handle) noexcept -> void {
    delete static_cast<opengl::Shader*>(handle);
}

static auto delete_opengl_material(void* handle) noexcept -> void {
    delete static_cast<opengl::Material*>(handle);
}

auto OpenGL_Renderer::set_clear_color(const RGBAColor& color) -> void {
    m_context.m_state.set_clear_color(color);
}

auto OpenGL_Renderer::clear_background() -> void {
    GLbitfield bitfield = m_context.m_state.clear_bitfield;
    glClear(bitfield);
}

auto OpenGL_Renderer::set_viewport(u32 x, u32 y, u32 width, u32 height) -> void {
    m_context.m_state.set_viewport(x, y, width, height);
}

static auto resource_error(std::string message) -> Result<void, InitError> {
    return err<void, InitError>(InitError{
        InitErrorCode::RESOURCE_CREATION_FAILED,
        GRAPHICS_API::OPENGL,
        std::move(message),
    });
}

OpenGL_Renderer::OpenGL_Renderer(RenderSystem& system, Window* window)
    : m_system(&system)
    , m_window(window) {}

auto OpenGL_Renderer::initialize() -> Result<void, InitError> {
    auto context = m_context.init(m_window);
    if (context.is_err()) { return context; }

#if JF_OPENGL_FB
    auto shading_code = GLSLCodeLoader::get_by_name("framebuffer_test");
    if (shading_code.is_err()) {
        return resource_error(
            std::string("cannot load the render target shader (") +
            to_string(shading_code.error().m_code) +
            "): " + shading_code.error().m_message
        );
    }
    NativeHandle shader = this->create_shader(shading_code.value());
    if (shader == nullptr) {
        return resource_error("cannot create the render target shader");
    }
    return m_render_target.init(&m_context, std::move(shader));
#else
    return ok<void, InitError>();
#endif
}

auto OpenGL_Renderer::create_buffer(
    GPUBuffer::TYPE type,
    const void*     data,
    size_t          size
) -> void* {
    if (size > std::numeric_limits<u32>::max()) {
        Logger::err("OpenGL buffer is too large: {} bytes", size);
        return nullptr;
    }
    return m_context.create_buffer(
        to_opengl(type), const_cast<void*>(data), static_cast<u32>(size)
    );
}

auto OpenGL_Renderer::destroy_buffer(void* handle) -> void {
    m_context.destroy_buffer(static_cast<opengl::Buffer*>(handle));
}

auto OpenGL_Renderer::create_texture(const Image& image) -> NativeHandle {
    const auto size = v2u32::create(
        static_cast<u32>(image.width), static_cast<u32>(image.height)
    );
    auto* texture = m_context.create_texture(
        const_cast<u8*>(image.data.data()), size, static_cast<u32>(image.num_components)
    );
    return NativeHandle(texture, noop_native_handle_deleter);
}

auto OpenGL_Renderer::create_shader(const ShadingCode& code) -> NativeHandle {
    if (code.m_modules.size() < 2) {
        Logger::err("OpenGL shaders require vertex and fragment modules");
        return NativeHandle(nullptr, noop_native_handle_deleter);
    }

    opengl::Shader::Desc shader_desc;
    shader_desc.shading_language = code.m_shading_language;
    shader_desc.code.m_shading_language = code.m_shading_language;
    shader_desc.code.m_modules.resize(2);

    for (size_t i = 0; i < 2; ++i) {
        const auto& module = code.m_modules[i];
        auto remapped = remap_for_opengl(module.m_code, module.m_stage, nullptr);
        if (remapped.is_err()) {
            Logger::err(
                "Cannot create the OpenGL shader ({}): {}",
                to_string(remapped.error().m_code),
                remapped.error().m_message
            );
            return NativeHandle(nullptr, noop_native_handle_deleter);
        }
        shader_desc.code.m_modules[i].m_code = std::move(remapped).value();
        shader_desc.code.m_modules[i].m_stage = module.m_stage;
    }

    return NativeHandle(
        new opengl::Shader(m_context, shader_desc), delete_opengl_shader
    );
}

auto OpenGL_Renderer::create_material(
    const ShaderHandle&  shader,
    const TextureHandle* texture
) -> NativeHandle {
    auto* native_shader = static_cast<opengl::Shader*>(shader.m_handle.get());
    auto* native_texture = texture == nullptr
                               ? nullptr
                               : static_cast<opengl::Texture*>(texture->m_handle.get());
    if (native_shader == nullptr || (texture != nullptr && native_texture == nullptr)) {
        Logger::err("Cannot create an OpenGL material from invalid resources");
        return NativeHandle(nullptr, noop_native_handle_deleter);
    }

    return NativeHandle(
        new opengl::Material(m_context, *native_shader, native_texture),
        delete_opengl_material
    );
}

auto OpenGL_Renderer::set_shader_uniform(
    void*              /*shader*/,
    const std::string& /*name*/,
    const void*        /*data*/,
    size_t             /*size*/
) -> Result<void, RendererError> {
    // The shaders are SPIR-V, which has no loose (default block) uniforms that
    // `glUniform*` could address by name. All uniform data lives in uniform blocks whose
    // buffers are owned by `opengl::Material` and written by `render()`.
    return err<void, RendererError>(RendererError{
        RendererErrorCode::UNSUPPORTED,
        "OpenGL: named shader uniforms are not supported, uniform data is provided "
        "through the uniform buffers of a material",
    });
}

auto OpenGL_Renderer::present() -> void {
    m_frame_rendered = false;
    m_context.m_swapchain_context.swap_buffers();
}

auto OpenGL_Renderer::wait_until_idle() -> void { glFinish(); }

auto OpenGL_Renderer::render(const Camera& camera) -> void {

#if JF_OPENGL_FB
    // The scene is drawn into an offscreen target with the client's viewport and then
    // copied 1:1 onto the window. That only works if the target has the size of the
    // window's framebuffer, so follow window resizes here.
    const v2u32 window_size = m_window->get_size();
    if (window_size.x > 0 && window_size.y > 0 &&
        (window_size.x != m_render_target.m_size.x ||
         window_size.y != m_render_target.m_size.y)) {
        m_render_target.resize(window_size);
    }
    m_context.bind_framebuffer(*m_render_target.m_framebuffer);
#endif

    this->clear_background();

    // NOTE: At the time of writing this is mainly compatible with
    // `get_shader_spirv_test_1` or rather on any renderer where the camera uniform is
    // at binding point 0 and the transform uniform is at binding point 1.
    const u32 CAM_BINDING = 0;
    const u32 TRANSFORM_BINDING = 1;

    std::deque<RenderCommand>& render_commands = m_system->m_render_commands;

    for (size_t i = 0; i < render_commands.size(); ++i) {
        const RenderCommand&  cmd = render_commands[i];
        const MaterialHandle& mh = *cmd.material;

        auto* material = static_cast<opengl::Material*>(mh.m_handle.get());
        auto* shader = static_cast<opengl::Shader*>(mh.m_shader->m_handle.get());
        m_context.bind_shader(*shader);

        // NOTE: As of right now this is not optimal, as it only needs to be updated once
        // outside the loop. But because of how the code is arranged one has to update it
        // every iteration of the loop. Later on one HAS TO fix this.

        // ub_cam
        mat4x4 view_projection = camera.get_view_projection(GRAPHICS_API::OPENGL);
        material->write_ub(CAM_BINDING, &view_projection, sizeof(view_projection), 0);

        // ub_tran
        material->write_ub(TRANSFORM_BINDING, &cmd.transform, sizeof(cmd.transform), 0);

        if (mh.m_texture != nullptr) {
            auto* texture = static_cast<opengl::Texture*>(mh.m_texture->m_handle.get());

            u32              texture_unit = 0;
            opengl::Sampler* sampler = m_context.m_default_sampler;
            m_context.m_texture_manager.bind_texture_and_sampler_to_unit(
                *texture, sampler, texture_unit
            );
        }

        OpenGL_Renderer::render_mesh(
            cmd.vertex_data, cmd.m_mesh, &shader->m_vertex_array
        );
    }
#if JF_OPENGL_FB
    m_context.unbind_framebuffer();
    m_context.m_state.set_depth_test(false);
    // The client's viewport was already applied while drawing into the offscreen
    // target. The copy onto the window has to cover the whole target, otherwise the
    // viewport would be applied a second time.
    const v2u32 viewport_pos = m_context.m_state.viewport[0];
    const v2u32 viewport_size = m_context.m_state.viewport[1];
    m_context.m_state.set_viewport(
        0, 0, m_render_target.m_size.x, m_render_target.m_size.y
    );
    m_render_target.render(m_system);
    m_context.m_state.set_viewport(
        viewport_pos.x, viewport_pos.y, viewport_size.x, viewport_size.y
    );
    m_context.m_state.set_depth_test(true);
#endif
#undef JF_OPENGL_FB
    render_commands.clear();
    m_frame_rendered = true;
}

auto OpenGL_Renderer::render_mesh(
    const Mesh*        vertex_data,
    const GPUMeshData* gpu_data,
    OGLW_VertexArray*  vao
) -> void {
    // TODO: Considering we are replicating Vulkan's way of doing things, the primitive
    // type should be defined in the pipeline or shader, and here it should be simply
    // queried.
    if (vao == nullptr || gpu_data == nullptr || vertex_data == nullptr) {
        Logger::err("Cannot render a mesh with incomplete render data");
        return;
    }
    auto validation = vertex_data->validate();
    if (validation.is_err()) {
        Logger::err("Cannot render an invalid mesh: {}", validation.error().m_message);
        return;
    }
    if (gpu_data->m_vertex_buffer == nullptr ||
        gpu_data->m_vertex_buffer->m_handle == nullptr) {
        Logger::err("Cannot render a mesh without a valid vertex buffer");
        return;
    }

    m_context.bind_vertex_array(*vao);
    auto* vertex_buffer =
        static_cast<opengl::Buffer*>(gpu_data->m_vertex_buffer->m_handle);
    vao->bind_buffer(*vertex_buffer);

    auto prim_type = static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES);
    if (!vertex_data->m_indices.empty()) {
        if (gpu_data->m_index_buffer == nullptr ||
            gpu_data->m_index_buffer->m_handle == nullptr) {
            Logger::err("Cannot render indexed mesh without an index buffer");
            return;
        }
        if (vertex_data->m_indices.size() >
            static_cast<size_t>(std::numeric_limits<GLsizei>::max())) {
            Logger::err("Mesh contains too many indices for OpenGL");
            return;
        }
        auto  num_indices = static_cast<GLsizei>(vertex_data->m_indices.size());
        auto  gl_type = GL_UNSIGNED_INT;
        auto* index_buffer =
            static_cast<opengl::Buffer*>(gpu_data->m_index_buffer->m_handle);
        glVertexArrayElementBuffer(vao->m_ID, index_buffer->m_id);
        glDrawElements(prim_type, num_indices, gl_type, nullptr);
    } else {
        const auto* position_attribute = vertex_data->attribute_data(Mesh::POSITION.m_id);
        if (position_attribute == nullptr ||
            position_attribute->count() >
                static_cast<size_t>(std::numeric_limits<GLsizei>::max())) {
            Logger::err("Mesh contains an invalid vertex count for OpenGL");
            return;
        }
        auto num_vertices = static_cast<GLsizei>(position_attribute->count());
        glDrawArrays(prim_type, 0, num_vertices);
    }
}

auto OpenGL_Renderer::take_screenshot(const char* filename)
    -> Result<Image, RendererError> {
    if (!m_frame_rendered) {
        // After a buffer swap the content of the back buffer is undefined.
        return err<Image, RendererError>(RendererError{
            RendererErrorCode::FRAME_NOT_AVAILABLE,
            "OpenGL: a screenshot can only be taken between render() and present()",
        });
    }

    // The viewport rectangle, clipped to the window's framebuffer. Reading outside of
    // the framebuffer would yield undefined values.
    const v2u32 window_size = m_window->get_size();
    const v2u32 viewport_pos = m_context.m_state.viewport[0];
    const v2u32 viewport_size = m_context.m_state.viewport[1];
    const u64   x0 = std::min<u64>(viewport_pos.x, window_size.x);
    const u64   y0 = std::min<u64>(viewport_pos.y, window_size.y);
    const u64   x1 = std::min<u64>(u64{viewport_pos.x} + viewport_size.x, window_size.x);
    const u64   y1 = std::min<u64>(u64{viewport_pos.y} + viewport_size.y, window_size.y);
    const u64   width = x1 - x0;
    const u64   height = y1 - y0;
    const u64   max_dimension = static_cast<u64>(std::numeric_limits<GLsizei>::max());
    if (width == 0 || height == 0 || width > max_dimension || height > max_dimension) {
        return err<Image, RendererError>(RendererError{
            RendererErrorCode::INVALID_STATE,
            "OpenGL: the viewport does not cover any part of the framebuffer",
        });
    }

    Image image;
    image.data.resize(width * height * 3);
    image.width = static_cast<i32>(width);
    image.height = static_cast<i32>(height);
    image.num_components = 3;

    // `glReadPixels` implicitly finishes the commands of the frame. Rows are returned
    // bottom-to-top, which is the row order of `Image`.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(
        static_cast<GLint>(x0),
        static_cast<GLint>(y0),
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height),
        GL_RGB,
        GL_UNSIGNED_BYTE,
        image.data.data()
    );
    if (m_context.m_bound_framebuffer != nullptr) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_context.m_bound_framebuffer->m_ID);
    }

    if (filename != nullptr && filename[0] != '\0' && !write_png(image, filename)) {
        return err<Image, RendererError>(RendererError{
            RendererErrorCode::IO_FAILURE,
            std::string("OpenGL: failed to write the screenshot to ") + filename,
        });
    }
    return ok<Image, RendererError>(std::move(image));
}

auto OpenGL_Renderer::RenderTarget::resize(v2u32 size) -> void {
    // Textures have immutable storage, so the color attachment has to be replaced. The
    // texture unit cache identifies textures by address and would not notice that.
    m_context->m_texture_manager.unbind_texture_from_unit(0);
    *m_texture = opengl::Texture(*m_context, nullptr, size, 3);
    m_renderbuffer->store(
        GL_DEPTH24_STENCIL8, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y)
    );
    m_framebuffer->attach(opengl::ATTACHMENT::COLOR, 0, *m_texture);
    m_framebuffer->attach(opengl::ATTACHMENT::DEPTH_STENCIL, 0, *m_renderbuffer);
    m_size = size;

    const GLenum res = m_framebuffer->check_status();
    if (res != GL_FRAMEBUFFER_COMPLETE) {
        Logger::err(
            "OpenGL render target is not complete after resizing to {}x{}, status: {}",
            size.x,
            size.y,
            framebuffer_res_to_str(res)
        );
    }
}

auto OpenGL_Renderer::RenderTarget::init(opengl::Context* context, NativeHandle shader)
    -> Result<void, InitError> {
    m_context = context;
    m_shader = std::move(shader);
    {
        // TODO: the use of `GL_RGB8`, `GL_RGB` and `GL_UNSIGNED_BYTE` is hardcoded. Find
        // a way to make it more flexible.
        const v2u32 size = context->m_state.viewport[1];
        m_size = size;

        m_texture = context->create_texture(nullptr, size, 3);
        m_sampler = context->create_sampler();
        m_sampler->set_parameters(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        m_sampler->set_parameters(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        m_sampler->set_parameters(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        m_sampler->set_parameters(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        m_renderbuffer = context->create_renderbuffer();
        m_renderbuffer->store(GL_DEPTH24_STENCIL8, size.x, size.y);

        m_framebuffer = context->create_framebuffer();
        m_framebuffer->attach(opengl::ATTACHMENT::COLOR, 0, *m_texture);
        m_framebuffer->attach(opengl::ATTACHMENT::DEPTH_STENCIL, 0, *m_renderbuffer);

        const GLenum res = m_framebuffer->check_status();
        if (res != GL_FRAMEBUFFER_COMPLETE) {
            return resource_error(
                "the " + std::to_string(size.x) + "x" + std::to_string(size.y) +
                " render target framebuffer is not complete, status: " +
                framebuffer_res_to_str(res)
            );
        }
    }

    MeshBuilder::Desc vdf_desc;
    vdf_desc.has_normals = false;

    const v2 viewport_pos = v2::create(-1.0F, -1.0F);
    const v2 viewport_size = v2::create(2.0F, 2.0F);

    auto mesh_result = MeshBuilder::rectangle_opengl(
        v3::create(viewport_pos.x, viewport_pos.y, 0.0F),
        v3::create(viewport_size.x, viewport_size.y, 0.0F),
        vdf_desc
    );
    if (mesh_result.is_err()) {
        return resource_error(
            "failed to create the render target mesh: " + mesh_result.error().m_message
        );
    }
    m_mesh = std::move(mesh_result).value();

    auto data_result = convert_into_data(m_mesh, true);
    if (data_result.is_err()) {
        return resource_error(
            "failed to convert the render target mesh: " + data_result.error().m_message
        );
    }
    std::vector<f32> data = std::move(data_result).value();
    u32              size = static_cast<u32>(data.size() * sizeof(f32));
    m_vertex_buffer =
        context->create_buffer(opengl::Buffer::TYPE::VERTEX, data.data(), size);
    if (m_vertex_buffer == nullptr || m_vertex_buffer->m_id == 0) {
        return resource_error("failed to create the render target vertex buffer");
    }
    return ok<void, InitError>();
}

auto OpenGL_Renderer::RenderTarget::render(RenderSystem* /*system*/) -> void {

    auto validation = m_mesh.validate();
    if (validation.is_err()) {
        Logger::err(
            "Cannot render the OpenGL render target mesh: {}",
            validation.error().m_message
        );
        return;
    }

    auto* sh = static_cast<opengl::Shader*>(m_shader.get());
    m_context->bind_shader(*sh);
    m_context->m_texture_manager.bind_texture_and_sampler_to_unit(
        *m_texture, m_sampler, 0
    );

    // this->render_mesh(m_mesh,)
    OGLW_VertexArray* vao = &sh->m_vertex_array;
    m_context->bind_vertex_array(*vao);
    vao->bind_buffer(*m_vertex_buffer);
    auto prim_type = static_cast<GLenum>(PRIMITIVE_TYPE::TRIANGLES);
    if (!m_mesh.m_indices.empty()) {
        if (m_mesh.m_indices.size() != 6) {
            Logger::err("The framebuffer mesh must have 6 indices");
            return;
        }
        auto num_indices = static_cast<GLsizei>(m_mesh.m_indices.size());
        auto gl_type = GL_UNSIGNED_INT;
        glDrawElements(prim_type, num_indices, gl_type, nullptr);
    } else {
        const auto* position_data = m_mesh.attribute_data(Mesh::POSITION.m_id);
        if (position_data == nullptr || position_data->count() != 6) {
            Logger::err("The framebuffer mesh must have 6 vertices");
            return;
        }
        GLsizei num_vertices = static_cast<GLsizei>(position_data->count());
        glDrawArrays(prim_type, 0, num_vertices);
    }
    // m_context->bind_vertex_array(*vao);
    // vao->bind_buffer(*m_vertex_buffer);

    // const GLsizei num_vertices = 6;
    // glDrawArrays(GL_TRIANGLES, 0, num_vertices);
}
} // namespace JadeFrame
