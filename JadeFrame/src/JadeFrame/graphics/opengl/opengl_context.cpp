#include "pch.h"
#include "opengl_context.h"
#include "opengl_debug.h"
#include "opengl_windows.h"

namespace JadeFrame {



OpenGL_Context::OpenGL_Context(const IWindow* window)
#ifdef WIN32
    : m_device_context(opengl::win32::init_device_context((const HWND)window->get())) {
    auto m_render_context = opengl::win32::init_render_context(m_device_context);
#elif __linux__
    {
    // : m_device_context(opengl::linux::init_device_context(window.m_window_handle)) {
    // auto m_render_context = opengl::linux::init_render_context(m_device_context);
#endif

    set_debug_mode(true);
    m_state.set_default();

    vendor = reinterpret_cast<char const*>(glGetString(GL_VENDOR));
    renderer = reinterpret_cast<char const*>(glGetString(GL_RENDERER));
    version = reinterpret_cast<char const*>(glGetString(GL_VERSION));
    shading_language_version = reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);


    // gather extentions
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (i32 i = 0; i < num_extensions; i++) {
        extentenions.push_back(reinterpret_cast<char const*>(glGetStringi(GL_EXTENSIONS, i)));
    }

    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &m_max_uniform_buffer_binding_points);

    // glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
    // glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
    // glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
    // glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);

    // opengl::win32::swap_interval(0); //TODO: This is windows specific. Abstract this away

    {
        const GLuint binding_point_0 = 0;
        m_uniform_buffers.emplace_back();
        m_uniform_buffers[0].bind();
        m_uniform_buffers[0].reserve(1 * sizeof(Matrix4x4));
        m_uniform_buffers[0].unbind();
        m_uniform_buffers[0].bind_base(binding_point_0);

        const GLuint binding_point_1 = 1;
        m_uniform_buffers.emplace_back();
        m_uniform_buffers[1].bind();
        m_uniform_buffers[1].reserve(1 * sizeof(Matrix4x4));
        m_uniform_buffers[1].unbind();
        m_uniform_buffers[1].bind_base(binding_point_1);
    }

    const v2u32& size = window->get_size();
    m_state.set_viewport(0, 0, size.x, size.y);
}

OpenGL_Context::~OpenGL_Context() {}


auto OpenGL_Context::swap_buffers() -> void {
#ifdef _WIN32
    ::SwapBuffers(m_device_context); // TODO: This is Windows specific. Abstract his away!
#endif
}


auto GL_State::set_default() -> void {
    this->set_clear_color({0.2f, 0.2f, 0.2f, 1.0f});
    this->set_depth_test(true);
    this->set_clear_bitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    this->set_blending(true);
    this->set_polygon_mode(POLYGON_FACE::FRONT_AND_BACK, POLYGON_MODE::FILL);
    this->set_face_culling(false, GL_BACK);
}

auto GL_State::set_blending(bool enable, BLENDING_FACTOR sfactor, BLENDING_FACTOR dfactor) -> void {
    if (blending != enable) {
        blending = enable;
        if (enable) {
            glEnable(GL_BLEND);
            glBlendFunc(sfactor, dfactor);
        } else {
            glDisable(GL_BLEND);
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

auto GL_State::set_clear_bitfield(const GLbitfield& bitfield) -> void { clear_bitfield = bitfield; }

auto GL_State::add_clear_bitfield(const GLbitfield& bitfield) -> void { clear_bitfield |= (1 << bitfield); }

auto GL_State::remove_clear_bitfield(const GLbitfield& bitfield) -> void { clear_bitfield &= ~(1 << bitfield); }

auto GL_State::set_depth_test(bool enable) -> void {
    if (depth_test != enable) {
        depth_test = enable;
        if (enable) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
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
    viewport[0] = {x, y};
    viewport[1] = {width, height};
    glViewport(x, y, width, height);
}
} // namespace JadeFrame