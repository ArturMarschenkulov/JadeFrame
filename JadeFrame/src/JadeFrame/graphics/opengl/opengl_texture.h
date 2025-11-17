#pragma once
#include <glad/glad.h>
#include "JadeFrame/math/vec.h"

namespace JadeFrame {
namespace opengl {

class Context;

class Sampler {
public:
    Sampler() noexcept = delete;

    Sampler(const Sampler&) = delete;
    auto operator=(const Sampler&) noexcept -> Sampler& = delete;

    Sampler(Sampler&& other) noexcept;
    auto operator=(Sampler&& other) noexcept -> Sampler&;
    explicit Sampler(opengl::Context& context);

    ~Sampler() noexcept;

    auto set_parameters(GLenum pname, GLenum param) const -> void;

public:
    GLuint   m_id = 0;
    Context* m_context = nullptr;
};

class Texture {
public:
    Texture() noexcept = delete;
    ~Texture() noexcept;

    Texture(const Texture&) = delete;
    auto operator=(const Texture&) noexcept -> Texture& = delete;

    Texture(Texture&& other) noexcept;
    auto operator=(Texture&& other) noexcept -> Texture&;

    Texture(opengl::Context& context, void* data, v2u32 size, u32 component_num);

    auto set_parameters(GLenum pname, GLenum param) const -> void;

private:
    auto generate_mipmap() const -> void;

    auto set_image(
        GLenum      internal_format,
        v2u32       size,
        GLenum      format,
        GLenum      type,
        const void* pixels
    ) -> void;

public:
    GLuint m_id = 0;

    GLenum m_internal_format;
    GLenum m_format;
    GLenum m_type;

    v2u32    m_size;
    Context* m_context = nullptr;
};
} // namespace opengl
} // namespace JadeFrame