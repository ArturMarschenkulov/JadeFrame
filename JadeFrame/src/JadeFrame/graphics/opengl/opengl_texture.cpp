
// #include "JadeFrame/prelude.h"

#include "JadeFrame/utils/logger.h"
#include "opengl_texture.h"
#include "opengl_context.h"

namespace JadeFrame {

namespace opengl {

Texture::Texture() noexcept {
    // glGenTextures(1, &m_id);
}

auto Texture::operator=(Texture&& other) noexcept -> Texture& {
    m_id = std::exchange(other.m_id, 0);
    return *this;
}

Texture::Texture(OpenGL_Context& context)
    : m_context(&context) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
}

Texture::Texture(OpenGL_Context& context, void* data, v2u32 size, u32 component_num)
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

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    Logger::warn("TextureHandle::init() - Texture created sjknmlml");
    /*
        GL_TEXTURE_WRAP_S and GL_TEXTURE_WRAP_T define how the texture should
       behave when the texture coordinates are outside the range [0, 1].
        - GL_REPEAT: The default behavior for textures. Repeats the texture
       image.
        - GL_MIRRORED_REPEAT: Same as GL_REPEAT but mirrors the image with each
       repeat.
        - GL_CLAMP_TO_EDGE: Clamps the coordinates between 0 and 1. The result
       is that higher coordinates become clamped to the edge, resulting in a
       stretched edge pattern.
        - GL_CLAMP_TO_BORDER: Coordinates outside the range are now given a
       user-specified border color.
        - GL_MIRROR_CLAMP_TO_EDGE: Almost the same as GL_MIRROR_CLAMP_TO_EDGE
       but it mirrors once more for a total of three repeats.

    */
    GLenum filter_min = GL_LINEAR;
    GLenum filter_max = GL_LINEAR; // GL_NEAREST;
    GLenum wrap_s = GL_REPEAT;
    GLenum wrap_t = GL_REPEAT;

    this->set_parameters(GL_TEXTURE_WRAP_S, wrap_s);
    this->set_parameters(GL_TEXTURE_WRAP_T, wrap_t);
    this->set_parameters(GL_TEXTURE_MIN_FILTER, filter_min);
    this->set_parameters(GL_TEXTURE_MAG_FILTER, filter_max);
    // this->set_texture_wrap_s(GL_REPEAT)
    // this->set_texture_wrap_t(GL_REPEAT)
    // this->set_texture_min_filter(GL_LINEAR)
    // this->set_texture_mag_filter(GL_LINEAR)

    this->set_image(0, internal_format, size, format, m_type, data);
    // if (m_mipmapping) {
    this->generate_mipmap();
    //}
}

Texture::Texture(Texture&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
    , m_internal_format(other.m_internal_format)
    , m_format(other.m_format)
    , m_type(other.m_type)
    , m_size(other.m_size) {}

Texture::~Texture() {
    glDeleteTextures(1, &m_id);
    m_id = 0;
}

auto Texture::generate_mipmap() const -> void { glGenerateTextureMipmap(m_id); }

auto Texture::set_parameters(GLenum pname, GLenum param) const -> void {
    glTextureParameteri(m_id, pname, param);
}

auto Texture::set_image(
    GLint       level,
    GLenum      internal_format,
    u32         size,
    GLenum      format,
    GLenum      type,
    const void* pixels
) -> void {
    assert(size > 0);
    i32 size_i = static_cast<i32>(size);
    glTextureStorage1D(m_id, 1, internal_format, size_i);
    glTextureSubImage1D(m_id, level, 0, size_i, format, type, pixels);
}

// https://docs.gl/gl4/glTexStorage2D
// https://docs.gl/gl4/glTexSubImage2D
// https://docs.gl/gl4/glTexImage2D
auto Texture::set_image(
    GLint       level,
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
    glTextureSubImage2D(m_id, level, 0, 0, size_x, size_y, format, type, pixels);
}

auto Texture::set_image(
    GLint       level,
    GLenum      internal_format,
    v3u32       size,
    GLenum      format,
    GLenum      type,
    const void* pixels
) -> void {
    assert(size.x > 0 && size.y > 0 && size.z > 0);
    i32 size_x = static_cast<i32>(size.x);
    i32 size_y = static_cast<i32>(size.y);
    i32 size_z = static_cast<i32>(size.z);
    glTextureStorage3D(m_id, 1, internal_format, size_x, size_y, size_z);
    glTextureSubImage3D(
        m_id, level, 0, 0, 0, size_x, size_y, size_z, format, type, pixels
    );
}

auto Texture::resize(u32 width, u32 height, u32 /*depth*/) -> void {

    assert(width > 0 && height > 0);
    this->set_image(
        0, m_internal_format, v2u32::create(width, height), m_format, m_type, 0
    );

    // switch (m_target) {
    //	case GL_TEXTURE_1D:
    //		assert(width > 0);
    //		m_texture.set_texture_image_1D(0, m_internal_format, width, 0,
    // m_format, m_type, 0); 		break; 	case GL_TEXTURE_2D:
    // assert(width > 0
    // && height > 0); 		m_texture.set_texture_image_2D(0, m_internal_format,
    // width, height, 0, m_format,
    // m_type,
    // 0); 		break; 	case GL_TEXTURE_3D: 		assert(width > 0 && height
    // >
    // 0
    // && depth > 0); 		m_texture.set_texture_image_3D(0,
    // m_internal_format, width, height, depth, 0, m_format, m_type, 0);
    // break; 	default: assert(false);
    // }
}

} // namespace opengl
} // namespace JadeFrame