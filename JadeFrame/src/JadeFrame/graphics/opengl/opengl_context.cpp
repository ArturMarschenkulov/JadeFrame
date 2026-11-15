#include "opengl_wrapper.h"

#if defined(_WIN32)
    #include "platform/win32/win32.h"
#elif defined(__linux__)
    #include "platform/linux/linux.h"
#endif

#include "opengl_context.h"
#include "JadeFrame/utils/logger.h"
#include "opengl_debug.h"
#include "opengl_shader.h"
#include <limits>

namespace JadeFrame {
namespace opengl {

auto TextureManager::create_texture(void* data, v2u32 size, u32 component_num)
    -> Texture* {
    if (!m_context || size.x == 0 || size.y == 0 ||
        size.x > static_cast<u32>(std::numeric_limits<GLsizei>::max()) ||
        size.y > static_cast<u32>(std::numeric_limits<GLsizei>::max()) ||
        (component_num != 3 && component_num != 4)) {
        return nullptr;
    }
    GLint maximum = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maximum);
    if (size.x > static_cast<u32>(maximum) || size.y > static_cast<u32>(maximum)) {
        return nullptr;
    }
    Texture texture(*m_context, data, size, component_num);
    if (!texture.m_id) { return nullptr; }
    m_textures.push_back(std::move(texture));
    return &m_textures.back();
}

auto TextureManager::create_sampler() -> Sampler* {
    m_samplers.emplace_back(*m_context);
    if (!m_samplers.back().m_id) {
        m_samplers.pop_back();
        return nullptr;
    }
    return &m_samplers.back();
}

auto TextureManager::bind_texture_and_sampler_to_unit(
    Texture& texture,
    Sampler* sampler,
    u32      unit
) -> void {
    // Search through `m_texture_units` and find `unit`.
    if (!m_texture_units.contains(unit)) {
        glBindTextureUnit(unit, texture.m_id);
        if (sampler != nullptr) { glBindSampler(unit, sampler->m_id); }
        m_texture_units[unit] = std::tuple(&texture, sampler);
    } else if (m_texture_units[unit] == std::tuple(&texture, sampler)) {
        return;
    } else {
        // // If the unit is already bound to another texture, unbind it first.
        // unbind_texture_from_unit(*m_texture_units[unit], unit);
        glBindTextureUnit(unit, texture.m_id);
        if (sampler != nullptr) { glBindSampler(unit, sampler->m_id); }
        m_texture_units[unit] = std::tuple(&texture, sampler);
    }
}

auto TextureManager::unbind_texture_from_unit(u32 unit) -> void {

    glBindTextureUnit(unit, 0);
    glBindSampler(unit, 0);
    m_texture_units[unit] = std::tuple(nullptr, nullptr);
}

auto Context::bind_uniform_buffer_to_location(opengl::Buffer& buffer, u32 location)
    -> void {
    if (buffer.m_type != opengl::Buffer::TYPE::UNIFORM) {
        Logger::err("Buffer is not of type uniform!");
        assert(false);
        return;
    }
    if (m_bound_uniform_buffer_locations.contains(location)) {
        if (m_bound_uniform_buffer_locations[location] == &buffer) { return; }
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, location, buffer.m_id);
    // glBindBufferRange(GL_UNIFORM_BUFFER, binding_point, buffer.m_id, 0,
    // buffer.m_size);
    m_bound_uniform_buffer_locations[location] = &buffer;
}

auto Context::bind_framebuffer(opengl::Framebuffer& framebuffer) -> void {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.m_ID);
    m_bound_framebuffer = &framebuffer;
}

auto Context::unbind_framebuffer() -> void {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_bound_framebuffer = nullptr;
}

auto Context::bind_renderbuffer(opengl::Renderbuffer& renderbuffer) -> void {
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer.m_ID);
    m_bound_renderbuffer = &renderbuffer;
}

auto Context::unbind_renderbuffer() -> void {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    m_bound_renderbuffer = nullptr;
}

auto Context::bind_shader(opengl::Shader& shader) -> void {
    assert(shader.m_program.m_ID != 0);
    if (m_bound_shader == &shader) { return; }
    glUseProgram(shader.m_program.m_ID);
    m_bound_shader = &shader;
}

auto Context::bind_vertex_array(OGLW_VertexArray& vao) -> void {
    assert(vao.m_ID != 0);
    if (m_bound_vertex_array == &vao) { return; }
    glBindVertexArray(vao.m_ID);
    m_bound_vertex_array = &vao;
}

auto Context::unbind_vertex_array() -> void {
    glBindVertexArray(0);
    m_bound_vertex_array = nullptr;
}

auto Context::bind_texture_to_unit(Texture& texture, u32 unit) -> void {
    m_texture_manager.bind_texture_and_sampler_to_unit(texture, nullptr, unit);
}

auto Context::unbind_texture_from_unit(u32 unit) -> void {
    m_texture_manager.unbind_texture_from_unit(unit);
}

auto Context::create_texture(void* data, v2u32 size, u32 component_num) -> Texture* {
    return m_texture_manager.create_texture(data, size, component_num);
}

auto Context::create_sampler() -> Sampler* { return m_texture_manager.create_sampler(); }

auto Context::create_buffer(opengl::Buffer::TYPE type, void* data, u32 size)
    -> opengl::Buffer* {
    if (!size ||
        ((type == Buffer::TYPE::VERTEX || type == Buffer::TYPE::INDEX) && !data)) {
        return nullptr;
    }
    auto buffer = opengl::Buffer::create(*this, type, data, size);
    if (!buffer.m_id) { return nullptr; }
    const auto id = buffer.m_id;
    auto [it, inserted] = m_bufferss.try_emplace(id, std::move(buffer));
    if (!inserted) {
        return nullptr;
    }
    m_buffers.push_back(it->second.m_id);
    m_bound_buffer = it->second.m_id;
    return &it->second;
}

auto Context::destroy_buffer(opengl::Buffer* buffer) -> void {
    if (buffer == nullptr) { return; }

    for (auto it = m_bufferss.begin(); it != m_bufferss.end(); ++it) {
        if (&it->second != buffer) { continue; }

        std::erase_if(m_bound_uniform_buffer_locations, [buffer](const auto& entry) {
            return entry.second == buffer;
        });
        std::erase(m_buffers, buffer->m_id);
        if (m_bound_buffer == buffer->m_id) { m_bound_buffer = 0; }
        m_bufferss.erase(it);
        return;
    }
}

auto Context::create_framebuffer() -> opengl::Framebuffer* {
    opengl::Framebuffer framebuffer(*this);
    if (!framebuffer.m_ID) { return nullptr; }
    m_framebuffers.push_back(std::move(framebuffer));
    return &m_framebuffers.back();
}

auto Context::create_renderbuffer() -> opengl::Renderbuffer* {
    opengl::Renderbuffer renderbuffer;
    if (!renderbuffer.m_ID) { return nullptr; }
    m_renderbuffers.push_back(std::move(renderbuffer));
    return &m_renderbuffers.back();
}

static auto context_error(std::string message) -> Result<void, InitError> {
    return err<void, InitError>(InitError{
        InitErrorCode::CONTEXT_CREATION_FAILED,
        GRAPHICS_API::OPENGL,
        std::move(message),
    });
}

static auto gl_string(GLenum name) -> std::string {
    const GLubyte* value = glGetString(name);
    return value == nullptr ? std::string() : reinterpret_cast<char const*>(value);
}

auto Context::init(Window* window) -> Result<void, InitError> {
    if (window == nullptr) {
        return err<void, InitError>(InitError{
            InitErrorCode::INVALID_ARGUMENT,
            GRAPHICS_API::OPENGL,
            "an OpenGL context cannot be created without a window",
        });
    }
#ifdef _WIN32
    auto* win = dynamic_cast<const JadeFrame::win32::NativeWindow*>(
        window->m_native_window.get()
    );
    if (win == nullptr) { return context_error("the window has no Win32 native window"); }

    // NOTE: This function might have to be moved, as in theory one could have
    // multiple contexts. NOTE: Think about removing the parameter from this function
    // then just using the global instance handle. loading wgl functions for render
    // context creation
    if (!opengl::win32::load_wgl_funcs(win->m_instance_handle)) {
        return context_error("failed to load the WGL functions");
    }

    m_swapchain_context.m_device_context = ::GetDC(win->m_window_handle);
    if (m_swapchain_context.m_device_context == nullptr) {
        return context_error("failed to get the device context of the window");
    }
    // Stored right away: from here on the destructor releases the render context.
    m_swapchain_context.m_render_context =
        opengl::win32::init_render_context(m_swapchain_context.m_device_context);
    if (m_swapchain_context.m_render_context == nullptr) {
        return context_error(
            "failed to create the WGL render context (error " +
            std::to_string(::GetLastError()) + ")"
        );
    }
    if (::wglMakeCurrent(
            m_swapchain_context.m_device_context, m_swapchain_context.m_render_context
        ) == FALSE) {
        return context_error(
            "failed to make the OpenGL context current on the window (error " +
            std::to_string(::GetLastError()) + ")"
        );
    }
    if (!opengl::win32::load_opengl_funcs(/*m_device_context, render_context*/)) {
        return context_error("failed to load the OpenGL functions");
    }

#elif __linux__

    // NOTE: This is weird. Somehwere the macro `linux` got defined.
    #undef linux
    auto* win = dynamic_cast<JadeFrame::X11_NativeWindow*>(window->m_native_window.get());
    if (win == nullptr) { return context_error("the window has no X11 native window"); }

    auto render_context = opengl::linux::create_glx_context(win);
    if (render_context.is_err()) {
        return err<void, InitError>(std::move(render_context).error());
    }
    // Stored right away: from here on the destructor releases the render context.
    m_swapchain_context.m_display = win->m_display;
    m_swapchain_context.m_window = win->m_window;
    m_swapchain_context.m_render_context = render_context.value();

    if (!opengl::linux::load_opengl_funcs()) {
        return context_error("failed to load the OpenGL functions");
    }
#else
    return err<void, InitError>(InitError{
        InitErrorCode::BACKEND_UNSUPPORTED,
        GRAPHICS_API::OPENGL,
        "the OpenGL renderer does not support this platform",
    });
#endif

    // The version is checked before anything else is done with the context: the rest
    // of the renderer uses OpenGL 4.5 functions (direct state access) unconditionally.
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    if (major_version < 4 || (major_version == 4 && minor_version < 5)) {
        return context_error(
            "OpenGL 4.5 or higher is required, the context provides " +
            std::to_string(major_version) + "." + std::to_string(minor_version)
        );
    }

    opengl::set_debug_mode(true);
    m_state.set_default();

    vendor = gl_string(GL_VENDOR);
    renderer = gl_string(GL_RENDERER);
    version = gl_string(GL_VERSION);
    shading_language_version = gl_string(GL_SHADING_LANGUAGE_VERSION);

    // At this point OpenGL is initialized
    m_texture_manager.m_context = this;

    Logger::info("OpenGL Vendor: {}", vendor);
    Logger::info("OpenGL Renderer: {}", renderer);
    Logger::info("OpenGL Version: {}", version);
    Logger::info("OpenGL Shading Language Version: {}", shading_language_version);
    Logger::info("OpenGL Version: {}.{}", major_version, minor_version);

    // gather extentions
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (i32 i = 0; i < num_extensions; i++) {
        const GLubyte* extension = glGetStringi(GL_EXTENSIONS, static_cast<u32>(i));
        if (extension == nullptr) { continue; }
        extentenions.emplace_back(reinterpret_cast<char const*>(extension));
    }

    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &m_max_uniform_buffer_binding_points);

    // glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
    // glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
    // glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
    // glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);

    // opengl::win32::swap_interval(0); //TODO: This is windows specific. Abstract
    // this away

    const v2u32& size = window->get_size();
    m_state.set_viewport(0, 0, size.x, size.y);

    GLint max_texture_units = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_texture_units);

    // TODO(artur): Consider where a better place to put this would be.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    m_default_sampler = this->create_sampler();
    if (!m_default_sampler) {
        return err<void, InitError>(InitError{
            InitErrorCode::RESOURCE_CREATION_FAILED,
            GRAPHICS_API::OPENGL,
            "failed to create the default texture sampler",
        });
    }
    return ok<void, InitError>();
}

Context::~Context() {
    m_bound_shader = nullptr;
    m_bound_vertex_array = nullptr;
    m_bound_renderbuffer = nullptr;
    m_bound_framebuffer = nullptr;
    m_bound_uniform_buffer_locations.clear();
    m_texture_manager.m_texture_units.clear();

    m_framebuffers.clear();
    m_renderbuffers.clear();
    m_vertex_arrays.clear();
    m_bufferss.clear();
    m_texture_manager.m_samplers.clear();
    m_texture_manager.m_textures.clear();

#ifdef _WIN32
    if (m_swapchain_context.m_render_context != nullptr) {
        ::wglMakeCurrent(nullptr, nullptr);
        ::wglDeleteContext(m_swapchain_context.m_render_context);
        m_swapchain_context.m_render_context = nullptr;
    }
#elif __linux__
    if (m_swapchain_context.m_display != nullptr &&
        m_swapchain_context.m_render_context != nullptr) {
        glXMakeCurrent(m_swapchain_context.m_display, None, nullptr);
        glXDestroyContext(
            m_swapchain_context.m_display, m_swapchain_context.m_render_context
        );
        m_swapchain_context.m_render_context = nullptr;
    }
#endif
}

auto SwapchainContext::swap_buffers() -> void {
#ifdef _WIN32
    ::SwapBuffers(m_device_context); // TODO: This is Windows specific. Abstract his away!
#elif __linux__
    glXSwapBuffers(m_display, m_window);
#endif
}

auto GL_State::set_default() -> void {
    this->set_clear_color(RGBAColor::from_rgba(0.2f, 0.2f, 0.2f, 1.0f));
    this->set_depth_test(true);
    this->set_clear_bitfield(
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT
    );
    this->set_blending(true);
    this->set_polygon_mode(POLYGON_FACE::FRONT_AND_BACK, POLYGON_MODE::FILL);
    this->set_face_culling(false, GL_BACK);
    glEnable(GL_FRAMEBUFFER_SRGB);
}

auto GL_State::set_blending(bool enable, BLENDING_FACTOR sfactor, BLENDING_FACTOR dfactor)
    -> void {
    if (blending != enable) {
        blending = enable;
        enable ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
        if (enable) {
            glBlendFunc(static_cast<GLenum>(sfactor), static_cast<GLenum>(dfactor));
        }
    }
}

auto GL_State::set_clear_color(const RGBAColor& color) -> void {
    if (clear_color != color) {
        clear_color = color;
        glClearColor(color.r, color.g, color.b, color.a);
    }
}

auto GL_State::set_polygon_mode(POLYGON_FACE face, POLYGON_MODE mode) -> void {
    if ((polygon_mode.first != face) || (polygon_mode.second != mode)) {
        polygon_mode = {face, mode};

        glPolygonMode((GLenum)face, (GLenum)mode);
    }
}

auto GL_State::set_clear_bitfield(const GLbitfield& bitfield) -> void {
    clear_bitfield = bitfield;
}

auto GL_State::add_clear_bitfield(const GLbitfield& bitfield) -> void {
    clear_bitfield |= bitfield;
}

auto GL_State::remove_clear_bitfield(const GLbitfield& bitfield) -> void {
    clear_bitfield &= ~bitfield;
}

auto GL_State::set_depth_test(bool enable) -> void {
    if (depth_test != enable) {
        depth_test = enable;
        enable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        // glDepthFunc(GL_LESS);
    }
}

auto GL_State::set_face_culling(bool enable, GLenum mode) -> void {
    if (is_face_culling != enable) {
        is_face_culling = enable;
        if (enable) {
            glEnable(GL_CULL_FACE);
            glCullFace(mode);
        } else {
            glDisable(GL_CULL_FACE);
        }
    }
}

auto GL_State::set_viewport(u32 x, u32 y, u32 width, u32 height) -> void {
    viewport[0] = v2u32::create(x, y);
    viewport[1] = v2u32::create(width, height);
    glViewport(
        static_cast<GLint>(x),
        static_cast<GLint>(y),
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height)
    );
}
} // namespace opengl
} // namespace JadeFrame
