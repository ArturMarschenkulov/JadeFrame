#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <utility>
#include "JadeFrame/graphics/mesh.h" // For Color
#include "JadeFrame/graphics/opengl/opengl_wrapper.h"
#include "opengl_buffer.h"

#ifdef _WIN32
    #include "JadeFrame/platform/windows/windows_window.h"
#elif __linux__
    #include "JadeFrame/platform/linux/linux_window.h"
    #include <GL/glx.h>
#endif

#ifdef _WIN32
struct HGLRC__;
using HGLRC = HGLRC__*;
struct HDC__;
using HDC = HDC__*;
#endif

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

struct Limits {
    GLint max_uniform_buffer_binding_points;
    GLint max_uniform_block_size;
    GLint max_vertex_uniform_blocks;
    GLint max_fragment_uniform_blocks;
};

class SwapchainContext {
public:
#ifdef WIN32
    HDC   m_device_context;
    HGLRC m_render_context;
#elif __linux__
    ::Display*    m_display = nullptr;
    ::GLXContext* m_render_context = nullptr;
    ::Window      m_window = 0;
#endif
    auto swap_buffers() -> void;
};

class OpenGL_Context {
public:
    OpenGL_Context() = default;
    explicit OpenGL_Context(const Window* window);
    ~OpenGL_Context();

public:
    SwapchainContext m_swapchain_context;

public:
    mutable GL_State m_state;

    template<typename T, typename U>
    using HashMap = std::unordered_map<T, U>;

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
    auto bind_texture_to_unit(opengl::Texture& texture, u32 unit) -> void;
    auto unbind_texture_from_unit(opengl::Texture& texture, u32 unit) -> void;
    std::unordered_map<u32, opengl::Texture*> m_texture_units;
    std::vector<opengl::Texture*>             m_textures = {};

    auto
    create_buffer(opengl::Buffer::TYPE type, void* data, u32 size) -> opengl::Buffer*;
    auto bind_uniform_buffer_to_location(opengl::Buffer& buffer, u32 location) -> void;

    std::unordered_map<u32, opengl::Buffer*> m_bound_uniform_buffer_locations;

    // std::vector<opengl::Buffer> m_uniform_buffers;

    std::vector<GLuint>          m_buffers;
    GLuint                       m_bound_buffer;
    HashMap<u32, opengl::Buffer> m_bufferss;

    auto create_framebuffer() -> opengl::Framebuffer*;
    auto bind_framebuffer(opengl::Framebuffer& framebuffer) -> void;
    auto unbind_framebuffer() -> void;

    opengl::Framebuffer* m_bound_framebuffer;

    auto create_renderbuffer() -> opengl::Renderbuffer*;
    auto bind_renderbuffer(opengl::Renderbuffer& renderbuffer) -> void;
    auto unbind_renderbuffer() -> void;

    opengl::Renderbuffer* m_bound_renderbuffer;

    auto bind_vertex_array(OGLW_VertexArray& vao) -> void;
    auto unbind_vertex_array() -> void;

    OGLW_VertexArray*              m_bound_vertex_array = nullptr;
    HashMap<u32, OGLW_VertexArray> m_vertex_arrays;

    auto bind_shader(opengl::Shader& shader) -> void;
    // auto unbind_shader() -> void {}
    // auto create_vertex_array(const VertexFormat& vertex_format);

    opengl::Shader* m_bound_shader = nullptr;
};
} // namespace JadeFrame