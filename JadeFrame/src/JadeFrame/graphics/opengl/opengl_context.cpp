#include "opengl_wrapper.h"

#if defined(_WIN32)
    #include "platform/win32/win32.h"
#elif defined(__linux__)
    #include "platform/linux/linux.h"
#endif

#include "opengl_context.h"
#include "opengl_debug.h"
#include "opengl_shader.h"

namespace JadeFrame {
auto OpenGL_Context::bind_uniform_buffer_to_location(opengl::Buffer& buffer, u32 location)
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
    // glBindBufferRange(GL_UNIFORM_BUFFER, binding_point, buffer.m_id, 0, buffer.m_size);
    m_bound_uniform_buffer_locations[location] = &buffer;
}

auto OpenGL_Context::bind_framebuffer(opengl::Framebuffer& framebuffer) -> void {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.m_ID);
    m_bound_framebuffer = &framebuffer;
}

auto OpenGL_Context::unbind_framebuffer() -> void {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_bound_framebuffer = nullptr;
}

auto OpenGL_Context::bind_renderbuffer(opengl::Renderbuffer& renderbuffer) -> void {
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer.m_ID);
    m_bound_renderbuffer = &renderbuffer;
}

auto OpenGL_Context::unbind_renderbuffer() -> void {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    m_bound_renderbuffer = nullptr;
}

auto OpenGL_Context::bind_shader(opengl::Shader& shader) -> void {
    assert(shader.m_program.m_ID != 0);
    if (m_bound_shader == &shader) { return; }
    glUseProgram(shader.m_program.m_ID);
    m_bound_shader = &shader;
}

auto OpenGL_Context::bind_vertex_array(OGLW_VertexArray& vao) -> void {
    assert(vao.m_ID != 0);
    if (m_bound_vertex_array == &vao) { return; }
    glBindVertexArray(vao.m_ID);
    m_bound_vertex_array = &vao;
}

auto OpenGL_Context::unbind_vertex_array() -> void {
    glBindVertexArray(0);
    m_bound_vertex_array = nullptr;
}

auto OpenGL_Context::bind_texture_to_unit(opengl::Texture& texture, u32 unit) -> void {
    // Search through `m_texture_units` and find `unit`.
    if (!m_texture_units.contains(unit)) {
        glBindTextureUnit(unit, texture.m_id);
        m_texture_units[unit] = &texture;
    } else if (m_texture_units[unit] == &texture) {
        return;
    } else {
        // // If the unit is already bound to another texture, unbind it first.
        // unbind_texture_from_unit(*m_texture_units[unit], unit);
        glBindTextureUnit(unit, texture.m_id);
        m_texture_units[unit] = &texture;
    }
}

auto OpenGL_Context::unbind_texture_from_unit(opengl::Texture& texture, u32 unit)
    -> void {

    glBindTextureUnit(unit, 0);
    m_texture_units[unit] = nullptr;
}

auto OpenGL_Context::create_texture() -> opengl::Texture* {
    return new opengl::Texture(*this);
}

auto OpenGL_Context::create_texture(void* data, v2u32 size, u32 component_num)
    -> opengl::Texture* {
    return new opengl::Texture(*this, data, size, component_num);
}

auto OpenGL_Context::create_buffer(opengl::Buffer::TYPE type, void* data, u32 size)
    -> opengl::Buffer* {
    static u32 id = 0;
    auto [it, inserted] =
        m_bufferss.try_emplace(id, opengl::Buffer::create(*this, type, data, size));
    if (!inserted) {
        Logger::warn("Buffer with id {} already exists!", id);
        assert(false);
        return nullptr;
    }
    id++;
    return &it->second;
}

auto OpenGL_Context::create_framebuffer() -> opengl::Framebuffer* {
    auto* buffer = new opengl::Framebuffer(*this);
    return buffer;
}

auto OpenGL_Context::create_renderbuffer() -> opengl::Renderbuffer* {
    auto* buffer = new opengl::Renderbuffer();
    return buffer;
}

OpenGL_Context::OpenGL_Context(Window* window) {
#ifdef WIN32
    auto* win = dynamic_cast<const JadeFrame::win32::NativeWindow*>(
        window->m_native_window.get()
    );

    // NOTE: This function might have to be moved, as in theory one could have multiple
    // contexts. NOTE: Think about removing the parameter from this function then just
    // using the global instance handle. loading wgl functions for render context creation
    opengl::win32::load_wgl_funcs(win->m_instance_handle);

    m_swapchain_context.m_device_context = ::GetDC(win->m_window_handle);
    m_swapchain_context.m_render_context =
        opengl::win32::init_render_context(m_swapchain_context.m_device_context);
    wglMakeCurrent(
        m_swapchain_context.m_device_context, m_swapchain_context.m_render_context
    );
    opengl::win32::load_opengl_funcs(/*m_device_context, render_context*/);

#elif __linux__

    // NOTE: This is weird. Somehwere the macro `linux` got defined.
    #undef linux
    #if !defined(linux)
    auto* win = dynamic_cast<JadeFrame::X11_NativeWindow*>(window->m_native_window.get());
    opengl::linux::load_glx_funcs(win);
    opengl::linux::load_opengl_funcs();
    m_swapchain_context.m_display = win->m_display;
    m_swapchain_context.m_window = win->m_window;
    #endif
#else
    {
#endif

    opengl::set_debug_mode(true);
    m_state.set_default();

    vendor = reinterpret_cast<char const*>(glGetString(GL_VENDOR));
    renderer = reinterpret_cast<char const*>(glGetString(GL_RENDERER));
    version = reinterpret_cast<char const*>(glGetString(GL_VERSION));
    shading_language_version =
        reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    if (major_version < 4 && minor_version < 5) {
        Logger::err("OpenGL version 4.5 or higher is required!");
        exit(EXIT_FAILURE);
    }

    // At this point OpenGL is initialized

    Logger::info("OpenGL Vendor: {}", vendor);
    Logger::info("OpenGL Renderer: {}", renderer);
    Logger::info("OpenGL Version: {}", version);
    Logger::info("OpenGL Shading Language Version: {}", shading_language_version);
    Logger::info("OpenGL Version: {}.{}", major_version, minor_version);

    // gather extentions
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (i32 i = 0; i < num_extensions; i++) {
        extentenions.emplace_back(
            reinterpret_cast<char const*>(
                glGetStringi(GL_EXTENSIONS, static_cast<u32>(i))
            )
        );
    }

    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &m_max_uniform_buffer_binding_points);

    // glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
    // glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
    // glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
    // glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);

    // opengl::win32::swap_interval(0); //TODO: This is windows specific. Abstract this
    // away

    const v2u32& size = window->get_size();
    m_state.set_viewport(0, 0, size.x, size.y);

    GLint max_texture_units = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_texture_units);
}

OpenGL_Context::~OpenGL_Context() {}

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
} // namespace JadeFrame