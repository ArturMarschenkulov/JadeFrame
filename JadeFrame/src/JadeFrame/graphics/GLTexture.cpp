#include "GLTexture.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

auto GLTexture::load(std::string path, GLenum target, GLenum internalFormat, bool srgb) -> GLTexture {
	GLTexture texture;
	texture.m_target = target;
	texture.m_internal_format = internalFormat;
	if (texture.m_internal_format == GL_RGB || texture.m_internal_format == GL_SRGB)
		texture.m_internal_format = srgb ? GL_SRGB : GL_RGB;
	if (texture.m_internal_format == GL_RGBA || texture.m_internal_format == GL_SRGB_ALPHA)
		texture.m_internal_format = srgb ? GL_SRGB_ALPHA : GL_RGBA;

	// flip textures on their y coordinate while loading
	stbi_set_flip_vertically_on_load(true);

	int width, height, num_components;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &num_components, 0);
	if (data) {
		GLenum format = -1;
		if (num_components == 1) {
			format = GL_RED;
		} else if (num_components == 3) {
			format = GL_RGB;
		} else if (num_components == 4) {
			format = GL_RGBA;
		}

		if (target == GL_TEXTURE_2D) {
			texture.generate(width, height, texture.m_internal_format, format, GL_UNSIGNED_BYTE, data);
		} else {
			__debugbreak();
		}
		stbi_image_free(data);
	} else {
		std::cout << "Texture failed to load at path: " + path << std::endl;
		stbi_image_free(data);
		return texture;
	}
	texture.m_width = width;
	texture.m_height = height;

	return texture;
}

void GLTexture::generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, void* data) {
		glGenTextures(1, &m_ID);

		m_width = width;
		m_height = height;
		m_depth = 0;
		m_internal_format = internalFormat;
		m_format = format;
		m_type = type;

		//assert(Target == GL_TEXTURE_2D);
		bind();
		glTexImage2D(m_target, 0, internalFormat, width, height, 0, format, type, data);
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_filterMin);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_filterMax);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_S, m_wrapS);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, m_wrapT);
		if (m_mipmapping) {
			glGenerateMipmap(m_target);
		}
		unbind();
	}
