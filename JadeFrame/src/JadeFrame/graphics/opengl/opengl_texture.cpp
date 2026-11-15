
// #include "JadeFrame/prelude.h"

#include "JadeFrame/graphics/opengl/opengl_wrapper.h"
#include "JadeFrame/utils/logger.h"
#include "opengl_texture.h"
#include "opengl_context.h"

namespace JadeFrame {

namespace opengl {

Sampler::Sampler(opengl::Context& context)
    : m_context(&context) {

    GLuint id = 0;
    glCreateSamplers(1, &id);
    m_id = id;

    if (!m_id) { return; }
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

    if (!size.x || !size.y || (component_num != 3 && component_num != 4)) { return; }
    GLenum internal_format = component_num == 3 ? GL_RGB8 : GL_RGBA8;
    GLenum format = component_num == 3 ? GL_RGB : GL_RGBA;
    m_internal_format = internal_format;
    m_format = format;
    m_type = GL_UNSIGNED_BYTE;
    // Scope GL's sticky error flag to this creation operation.
    while (glGetError() != GL_NO_ERROR) {}
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    if (!m_id) { return; }
    this->set_image(internal_format, size, format, m_type, data);
    GLint allocated_width = 0;
    GLint allocated_height = 0;
    glGetTextureLevelParameteriv(m_id, 0, GL_TEXTURE_WIDTH, &allocated_width);
    glGetTextureLevelParameteriv(m_id, 0, GL_TEXTURE_HEIGHT, &allocated_height);
    if (allocated_width != static_cast<GLint>(size.x) ||
        allocated_height != static_cast<GLint>(size.y) || glGetError() != GL_NO_ERROR) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
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
    if (pixels) {
        // Image uses tightly packed rows, including RGB widths not divisible by four.
        GLint alignment = 4;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTextureSubImage2D(m_id, 0, 0, 0, size_x, size_y, format, type, pixels);
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    }
}

} // namespace opengl
} // namespace JadeFrame
