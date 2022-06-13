#pragma once
#include "JadeFrame/prelude.h"
#include <glad/glad.h>
#include "opengl_wrapper.h"
#include "JadeFrame/math/vec.h"

#include <string>

namespace JadeFrame {

class OpenGL_Texture;
class OpenGL_Texture {
public:
    OpenGL_Texture(void* data, v2u32 size, GLenum internal_format, GLenum format, GLenum type);
    auto resize(u32 width, u32 height, u32 depth) -> void;
    auto bind() const -> void;
    auto unbind() const -> void;

public:
    OGLW_Texture<GL_TEXTURE_2D> m_texture;

    const GLenum m_internal_format;
    const GLenum m_format;
    const GLenum m_type;

    const v2u32 m_size;
    // const GLuint m_width;
    // const GLuint m_height;
};

} // namespace JadeFrame