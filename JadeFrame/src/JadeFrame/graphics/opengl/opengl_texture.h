#pragma once
#include "JadeFrame/prelude.h"
#include <glad/glad.h>
#include "JadeFrame/math/vec.h"

namespace JadeFrame {
class OpenGL_Context;

namespace opengl {

class Texture {
public:
    // auto operator=(Texture&&) noexcept -> Texture& = delete;

private:
    auto release() -> GLuint;
    auto reset(GLuint id = 0) -> void;

public:
    Texture() noexcept;
    ~Texture();

    Texture(const Texture&) = delete;
    auto operator=(const Texture&) noexcept -> Texture& = delete;

    Texture(Texture&& other) noexcept;
    auto operator=(Texture&& other) noexcept -> Texture&;

    explicit Texture(OpenGL_Context& context);
    Texture(OpenGL_Context& context, void* data, v2u32 size, u32 component_num);

    auto        resize(u32 width, u32 height, u32 depth) -> void;
    auto        bind(u32 unit) const -> void;
    static auto unbind() -> void;

    auto generate_mipmap() const -> void;
    auto set_parameters(GLenum pname, GLint param) const -> void;
    auto set_image(
        GLint       level,
        GLint       internalformat,
        u32         size,
        GLint       border,
        GLenum      format,
        GLenum      type,
        const void* pixels
    ) -> void;
    auto set_image(
        GLint       level,
        GLint       internalformat,
        v2u32       size,
        GLint       border,
        GLenum      format,
        GLenum      type,
        const void* pixels
    ) -> void;
    auto set_image(
        GLint       level,
        GLint       internalformat,
        v3u32       size,
        GLint       border,
        GLenum      format,
        GLenum      type,
        const void* pixels
    ) -> void;

public:
    GLuint m_id;

    GLenum m_internal_format;
    GLenum m_format;
    GLenum m_type;

    v2u32 m_size;
    // const GLuint m_width;
    // const GLuint m_height;
    OpenGL_Context* m_context;
};
} // namespace opengl
} // namespace JadeFrame