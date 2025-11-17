
// #include "JadeFrame/prelude.h"

#include "JadeFrame/graphics/opengl/opengl_wrapper.h"
#include "JadeFrame/utils/logger.h"
#include "opengl_texture.h"
#include "opengl_context.h"

namespace JadeFrame {

static auto calc_mip_levels(u32 w, u32 h) -> u32 {
    u32 max_dim = (w > h) ? w : h;
    u32 levels = 1;
    while (max_dim > 1) {
        max_dim >>= 1U;
        ++levels;
    }
    return levels;
}

namespace opengl {

Sampler::Sampler(opengl::Context& context)
    : m_context(&context) {

    GLuint id = 0;
    glCreateSamplers(1, &id);
    m_id = id;

    this->set_parameters(GL_TEXTURE_WRAP_S, GL_REPEAT);
    this->set_parameters(GL_TEXTURE_WRAP_T, GL_REPEAT);
    this->set_parameters(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    this->set_parameters(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Sampler::~Sampler() noexcept {
    if (m_id != 0) { glDeleteSamplers(1, &m_id); }
}

auto Sampler::set_parameters(GLenum pname, GLenum param) const -> void {
    glSamplerParameteri(m_id, pname, param);
}

Texture::Texture(opengl::Context& context, void* data, v2u32 size, u32 component_num)
    : m_size(size)
    , m_context(&context) {

    GLenum internal_format = {};
    switch (component_num) {
        case 3: internal_format = GL_RGB8; break;
        case 4: internal_format = GL_RGBA8; break;
        default:
            Logger::err(
                "TextureHandle::init() - Unsupported number of components: {}",
                component_num
            );
            assert(false);
    }

    GLenum format = {};
    switch (component_num) {
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default:
            Logger::err(
                "TextureHandle::init() - Unsupported number of components: {}",
                component_num
            );
            assert(false);
    }
    m_internal_format = internal_format;
    m_format = format;
    m_type = GL_UNSIGNED_BYTE;
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    this->set_image(internal_format, size, format, m_type, data);
    // if (m_mipmapping) {
    //     this->generate_mipmap();
    // }
}

Texture::Texture(Texture&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
    , m_internal_format(other.m_internal_format)
    , m_format(other.m_format)
    , m_type(other.m_type)
    , m_size(other.m_size)
    , m_context((other.m_context)) {
    other.m_size = {};
    other.m_context = nullptr;
}

auto Texture::operator=(Texture&& other) noexcept -> Texture& {
    if (this == &other) { return *this; }
    if (m_id != 0) { glDeleteTextures(1, &m_id); }
    m_id = std::exchange(other.m_id, 0);
    m_internal_format = other.m_internal_format;
    m_format = other.m_format;
    m_type = other.m_type;
    m_size = other.m_size;
    m_context = other.m_context;

    other.m_size = {};
    other.m_context = nullptr;
    return *this;
}

Texture::~Texture() noexcept {
    if (m_id != 0) { glDeleteTextures(1, &m_id); }
}

auto Texture::generate_mipmap() const -> void { glGenerateTextureMipmap(m_id); }

auto Texture::set_parameters(GLenum pname, GLenum param) const -> void {
    glTextureParameteri(m_id, pname, param);
}

// https://docs.gl/gl4/glTexStorage2D
// https://docs.gl/gl4/glTexSubImage2D
// https://docs.gl/gl4/glTexImage2D
auto Texture::set_image(
    GLenum      internal_format,
    v2u32       size,
    GLenum      format,
    GLenum      type,
    const void* pixels
) -> void {
    assert(size.x > 0 && size.y > 0);
    i32 size_x = static_cast<i32>(size.x);
    i32 size_y = static_cast<i32>(size.y);
    glTextureStorage2D(m_id, 1, internal_format, size_x, size_y);
    glTextureSubImage2D(m_id, 0, 0, 0, size_x, size_y, format, type, pixels);
}

} // namespace opengl
} // namespace JadeFrame