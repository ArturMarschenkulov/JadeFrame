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
#endif

struct HGLRC__;
typedef HGLRC__* HGLRC;
struct HDC__;
typedef HDC__* HDC;

namespace JadeFrame {

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
    auto set_blending(bool enable, BLENDING_FACTOR sfactor = SRC_ALPHA, BLENDING_FACTOR dfactor = ONE_MINUS_SRC_ALPHA)
        -> void;
    auto set_clear_color(const RGBAColor& color) -> void;
    auto set_polygon_mode(POLYGON_FACE face, POLYGON_MODE mode) -> void;
    auto set_clear_bitfield(const GLbitfield& bitfield) -> void;
    auto add_clear_bitfield(const GLbitfield& bitfield) -> void;
    auto remove_clear_bitfield(const GLbitfield& bitfield) -> void;
    auto set_depth_test(bool enable) -> void;
    auto set_face_culling(bool enable, GLenum mode) -> void; // mode = GL_FRONT, GL_BACK, and GL_FRONT_AND_BACK

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
    OpenGL_Context(const IWindow* window);
    ~OpenGL_Context();

public:
#ifdef WIN32
    HDC m_device_context; // NOTE: Windows specific!
#elif __linux__
#endif
    auto swap_buffers() -> void;

// Resource creation
    auto create_texture() -> opengl::Texture;
    auto create_texture(void* data, v2u32 size, u32 component_num) -> opengl::Texture;


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

    // std::vector<opengl::Buffer> m_uniform_buffers;

    std::vector<GLuint> m_buffers;
    GLuint              m_bound_buffer;

    std::vector<u32> m_textures;
    std::vector<u32> m_texture_units;
    u32              m_active_texture_unit;
};
} // namespace JadeFrame