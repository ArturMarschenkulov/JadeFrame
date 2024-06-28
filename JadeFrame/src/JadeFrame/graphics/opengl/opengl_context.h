#pragma once
#include <glad/glad.h>
#include <string>
#include <utility>
#include "JadeFrame/graphics/mesh.h" // For Color
#include "opengl_buffer.h"

#ifdef _WIN32
    #include "JadeFrame/platform/windows/windows_window.h"
#elif __linux__
    #include "JadeFrame/platform/linux/linux_window.h"
    #include <GL/glx.h>
#endif

struct HGLRC__;
typedef HGLRC__* HGLRC;
struct HDC__;
typedef HDC__* HDC;

namespace JadeFrame {

namespace opengl {
class Shader;
}

enum BLENDING_FACTOR : i32 {
    ZERO = GL_ZERO,
    ONE = GL_ONE,
    SRC_COLOR = GL_SRC_COLOR,
    ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
    DST_COLOR = GL_DST_COLOR,
    ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
    SRC_ALPHA = GL_SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
    DST_ALPHA = GL_DST_ALPHA,
    ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
    CONSTANT_COLOR = GL_CONSTANT_COLOR,
    ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
    CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
    ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
};
enum class POLYGON_FACE : GLenum {
    FRONT_AND_BACK = GL_FRONT_AND_BACK,
    FRONT = GL_FRONT,
    BACK = GL_BACK,
};
enum class POLYGON_MODE : GLenum {
    POINT = GL_POINT,
    LINE = GL_LINE,
    FILL = GL_FILL,
};

struct GL_State {
public:
    auto set_default() -> void;
    auto set_blending(
        bool            enable,
        BLENDING_FACTOR sfactor = SRC_ALPHA,
        BLENDING_FACTOR dfactor = ONE_MINUS_SRC_ALPHA
    ) -> void;
    auto set_clear_color(const RGBAColor& color) -> void;
    auto set_polygon_mode(POLYGON_FACE face, POLYGON_MODE mode) -> void;
    auto set_clear_bitfield(const GLbitfield& bitfield) -> void;
    auto add_clear_bitfield(const GLbitfield& bitfield) -> void;
    auto remove_clear_bitfield(const GLbitfield& bitfield) -> void;
    auto set_depth_test(bool enable) -> void;
    auto set_face_culling(bool enable, GLenum mode)
        -> void; // mode = GL_FRONT, GL_BACK, and GL_FRONT_AND_BACK

public:
    bool                                  depth_test;
    RGBAColor                             clear_color;
    GLbitfield                            clear_bitfield;
    bool                                  blending;
    bool                                  is_face_culling;
    GLenum                                face_culling_mode;
    std::pair<POLYGON_FACE, POLYGON_MODE> polygon_mode;

public:
    auto  set_viewport(u32 x, u32 y, u32 width, u32 height) -> void;
    v2u32 viewport[2]; // TODO: Create an appropriate "rectangle" struct!
};

class OpenGL_Context {
public:
    OpenGL_Context() = default;
    explicit OpenGL_Context(const Window* window);
    ~OpenGL_Context();

public:
#ifdef WIN32
    HDC   m_device_context; // NOTE: Windows specific!
    HGLRC m_render_context;
#elif __linux__
    ::Display*    m_display = nullptr;
    ::GLXContext* m_render_context = nullptr;
    ::Window      m_window;
#endif
    auto swap_buffers() -> void;

public:
    mutable GL_State m_state;

    std::string              vendor;
    std::string              renderer;
    std::string              version;
    std::string              shading_language_version;
    std::vector<std::string> extentenions;
    i32                      major_version;
    i32                      minor_version;
    i32                      num_extensions;

    // limits
    GLint m_max_uniform_buffer_binding_points;

    // Resource creation
    auto create_texture() -> opengl::Texture*;
    auto create_texture(void* data, v2u32 size, u32 component_num) -> opengl::Texture*;
    auto create_buffer(opengl::Buffer::TYPE type, void* data, u32 size)
        -> opengl::Buffer*;
    auto create_framebuffer() -> opengl::Framebuffer*;
    auto create_renderbuffer() -> opengl::Renderbuffer*;

    auto bind_texture(opengl::Texture& texture, u32 unit) -> void;
    auto unbind_texture() -> void;

    opengl::Texture* m_bound_texture = nullptr;

    auto bind_vertex_array(OGLW_VertexArray& vao) -> void;
    auto unbind_vertex_array() -> void;

    OGLW_VertexArray* m_bound_vertex_array = nullptr;

    auto bind_shader(opengl::Shader& shader) -> void;
    // auto unbind_shader() -> void {}

    opengl::Shader* m_bound_shader = nullptr;

    auto bind_framebuffer(opengl::Framebuffer& framebuffer) -> void;
    auto unbind_framebuffer() -> void;

    opengl::Framebuffer* m_bound_framebuffer;

    // std::vector<opengl::Buffer> m_uniform_buffers;

    std::vector<GLuint> m_buffers;
    GLuint              m_bound_buffer;

    std::vector<opengl::Texture*> m_textures = {};
    std::vector<u32>              m_texture_units;

    template<typename T, typename U>
    using HashMap = std::unordered_map<T, U>;

    HashMap<u32, opengl::Buffer>   m_bufferss;
    HashMap<u32, OGLW_VertexArray> m_vertex_arrays;
};
} // namespace JadeFrame