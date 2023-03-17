#include "pch.h"

#include "opengl_texture.h"




#include "JadeFrame/prelude.h"
#include "opengl_context.h"
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {



namespace opengl {

Texture::Texture() noexcept {
    // glGenTextures(1, &m_id);
}
auto Texture::operator=(Texture&& other) noexcept -> Texture& {
    m_id = other.release();
    return *this;
}
Texture::Texture(OpenGL_Context& context) { glCreateTextures(GL_TEXTURE_2D, 1, &m_id); }
Texture::Texture(OpenGL_Context& context, void* data, v2u32 size, u32 component_num)
    : m_size(size) {
    m_context = &context;
    GLenum format_ = {};
    switch (component_num) {
        case 3: format_ = GL_RGB; break;
        case 4: format_ = GL_RGBA; break;
        default:
            Logger::err("TextureHandle::init() - Unsupported number of components: {}", component_num);
            assert(false);
    }
    m_internal_format = format_;
    m_format = format_;
    m_type = GL_UNSIGNED_BYTE;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    Logger::warn("TextureHandle::init() - Texture created sjknmlml");

    this->bind(0);



    /*
        GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T define how the texture should behave when the texture coordinates are
            outside the range [0, 1].
        - GL_REPEAT: The default behavior for textures. Repeats the texture image.
        - GL_MIRRORED_REPEAT: Same as GL_REPEAT but mirrors the image with each repeat.
        - GL_CLAMP_TO_EDGE: Clamps the coordinates between 0 and 1. The result is that higher coordinates become clamped
            to the edge, resulting in a stretched edge pattern.
        - GL_CLAMP_TO_BORDER: Coordinates outside the range are now given a user-specified border color.
        - GL_MIRROR_CLAMP_TO_EDGE: Almost the same as GL_MIRROR_CLAMP_TO_EDGE but it
            mirrors once more for a total of three repeats.

    */
    GLenum filter_min = GL_LINEAR;
    GLenum filter_max = GL_LINEAR; // GL_NEAREST;
    GLenum wrap_s = GL_REPEAT;
    GLenum wrap_t = GL_REPEAT;

    this->set_parameters(GL_TEXTURE_WRAP_S, wrap_s);
    this->set_parameters(GL_TEXTURE_WRAP_T, wrap_t);
    this->set_parameters(GL_TEXTURE_MIN_FILTER, filter_min);
    this->set_parameters(GL_TEXTURE_MAG_FILTER, filter_max);

    this->set_image(0, format_, size, 0, format_, m_type, data);
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
auto Texture::set_parameters(GLenum pname, GLint param) const -> void { glTextureParameteri(m_id, pname, param); }
auto Texture::set_image(
    GLint level, GLint internalformat, u32 size, GLint border, GLenum format, GLenum type, const void* pixels) -> void {
    glTexImage1D(GL_TEXTURE_2D, level, internalformat, size, border, format, type, pixels);
}
auto Texture::set_image(
    GLint level, GLint internalformat, v2u32 size, GLint border, GLenum format, GLenum type, const void* pixels)
    -> void {
    glTexImage2D(GL_TEXTURE_2D, level, internalformat, size.x, size.y, border, format, type, pixels);
}
auto Texture::set_image(
    GLint level, GLint internalformat, v3u32 size, GLint border, GLenum format, GLenum type, const void* pixels)
    -> void {
    glTexImage3D(GL_TEXTURE_2D, level, internalformat, size.x, size.y, size.z, border, format, type, pixels);
}
auto Texture::resize(u32 width, u32 height, u32 /*depth*/) -> void {

    this->bind(0);
    assert(width > 0 && height > 0);
    this->set_image(0, m_internal_format, {width, height}, 0, m_format, m_type, 0);

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
auto Texture::unbind() const -> void { glBindTexture(GL_TEXTURE_2D, 0); }


} // namespace opengl
} // namespace JadeFrame