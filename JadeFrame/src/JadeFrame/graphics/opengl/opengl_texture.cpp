#include "pch.h"

#include "opengl_texture.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "JadeFrame/prelude.h"

namespace JadeFrame {

struct STBIImage {
    STBIImage(const std::string& path) {
        // flip textures on their y coordinate while loading
        stbi_set_flip_vertically_on_load(true);
        // i32 width, height, num_components;
        data = stbi_load(path.c_str(), &width, &height, &num_components, 0);
    }
    ~STBIImage() { stbi_image_free(data); }
    i32            width, height, num_components;
    unsigned char* data;
};
namespace opengl {

Texture::Texture() noexcept { glGenTextures(1, &m_id); }
Texture::Texture(void* data, v2u32 size, GLenum internal_format, GLenum format, GLenum type)
    : m_size(size)
    , m_internal_format(internal_format)
    , m_format(format)
    , m_type(type) {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);

    this->bind(0);

    GLenum filter_min = GL_LINEAR;
    GLenum filter_max = GL_LINEAR; // GL_NEAREST;
    GLenum wrap_s = GL_REPEAT;
    GLenum wrap_t = GL_REPEAT;

    this->set_texture_parameters(GL_TEXTURE_WRAP_S, wrap_s);
    this->set_texture_parameters(GL_TEXTURE_WRAP_T, wrap_t);
    this->set_texture_parameters(GL_TEXTURE_MIN_FILTER, filter_min);
    this->set_texture_parameters(GL_TEXTURE_MAG_FILTER, filter_max);

    this->set_texture_image(0, internal_format, size, 0, format, type, data);
    // if (m_mipmapping) {
    this->generate_mipmap();
    //}
    this->unbind();
}
Texture::Texture(Texture&& other) noexcept
    : m_id(other.release())
    , m_size(other.m_size)
    , m_internal_format(other.m_internal_format)
    , m_format(other.m_format)
    , m_type(other.m_type) {}

Texture::~Texture() { this->reset(); }

auto Texture::generate_mipmap() const -> void { glGenerateTextureMipmap(m_id); }
auto Texture::set_texture_parameters(GLenum pname, GLint param) const -> void {
    glTextureParameteri(m_id, pname, param);
}
auto Texture::set_texture_image(
    GLint level, GLint internalformat, u32 size, GLint border, GLenum format, GLenum type, const void* pixels) -> void {
    glTexImage1D(GL_TEXTURE_2D, level, internalformat, size, border, format, type, pixels);
}
auto Texture::set_texture_image(
    GLint level, GLint internalformat, v2u32 size, GLint border, GLenum format, GLenum type, const void* pixels)
    -> void {
    glTexImage2D(GL_TEXTURE_2D, level, internalformat, size.x, size.y, border, format, type, pixels);
}
auto Texture::set_texture_image(
    GLint level, GLint internalformat, v3u32 size, GLint border, GLenum format, GLenum type, const void* pixels)
    -> void {
    glTexImage3D(GL_TEXTURE_2D, level, internalformat, size.x, size.y, size.z, border, format, type, pixels);
}
auto Texture::resize(u32 width, u32 height, u32 /*depth*/) -> void {

    this->bind(0);
    assert(width > 0 && height > 0);
    this->set_texture_image(0, m_internal_format, {width, height}, 0, m_format, m_type, 0);

    // switch (m_target) {
    //	case GL_TEXTURE_1D:
    //		assert(width > 0);
    //		m_texture.set_texture_image_1D(0, m_internal_format, width, 0, m_format, m_type, 0);
    //		break;
    //	case GL_TEXTURE_2D:
    //		assert(width > 0 && height > 0);
    //		m_texture.set_texture_image_2D(0, m_internal_format, width, height, 0, m_format, m_type, 0);
    //		break;
    //	case GL_TEXTURE_3D:
    //		assert(width > 0 && height > 0 && depth > 0);
    //		m_texture.set_texture_image_3D(0, m_internal_format, width, height, depth, 0, m_format, m_type, 0);
    //		break;
    //	default: assert(false);
    // }
}

auto Texture::release() -> GLuint {
    GLint id = m_id;
    m_id = 0;
    return id;
}
auto Texture::reset(GLuint id) -> void {
    glDeleteTextures(1, &m_id);
    m_id = id;
}
auto Texture::bind(u32 unit) const -> void {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}
auto Texture::unbind() const -> void {
    glBindTexture(GL_TEXTURE_2D, 0);
}


} // namespace opengl
} // namespace JadeFrame