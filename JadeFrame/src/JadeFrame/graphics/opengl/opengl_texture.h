#pragma once
#include <glad/glad.h>
#include "JadeFrame/math/vec.h"

namespace JadeFrame {
namespace opengl {

class Context;

class Texture {
public:
    // auto operator=(Texture&&) noexcept -> Texture& = delete;

public:
    Texture() noexcept;
    ~Texture();

    Texture(const Texture&) = delete;
    auto operator=(const Texture&) noexcept -> Texture& = delete;

    Texture(Texture&& other) noexcept;
    auto operator=(Texture&& other) noexcept -> Texture&;

    explicit Texture(opengl::Context& context);
    Texture(opengl::Context& context, void* data, v2u32 size, u32 component_num);

    auto resize(u32 width, u32 height, u32 depth) -> void;

    auto generate_mipmap() const -> void;
    auto set_parameters(GLenum pname, GLenum param) const -> void;
    auto set_image(
        GLint       level,
        GLenum      internal_format,
        u32         size,
        GLenum      format,
        GLenum      type,
        const void* pixels
    ) -> void;
    auto set_image(
        GLint       level,
        GLenum      internal_format,
        v2u32       size,
        GLenum      format,
        GLenum      type,
        const void* pixels
    ) -> void;
    auto set_image(
        GLint       level,
        GLenum      internal_format,
        v3u32       size,
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
    Context* m_context = nullptr;
};
} // namespace opengl
} // namespace JadeFrame