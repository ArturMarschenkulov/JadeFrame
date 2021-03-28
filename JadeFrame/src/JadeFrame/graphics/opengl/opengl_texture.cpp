#include "opengl_texture.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <iostream>

auto GLTextureLoader::load(const std::string& path, GLenum target, GLenum internalFormat, bool is_srgb) -> GLTexture {
	GLTexture texture;
	texture.m_target = target;
	texture.m_internal_format = internalFormat;
	if (texture.m_internal_format == GL_RGB || texture.m_internal_format == GL_SRGB) {
		texture.m_internal_format = is_srgb ? GL_SRGB : GL_RGB;
	}
	if (texture.m_internal_format == GL_RGBA || texture.m_internal_format == GL_SRGB_ALPHA) {
		texture.m_internal_format = is_srgb ? GL_SRGB_ALPHA : GL_RGBA;
	}

	// flip textures on their y coordinate while loading
	stbi_set_flip_vertically_on_load(true);

	int width, height, num_components;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &num_components, 0);
	if (data) {
		GLenum format = -1;
		//if (num_components == 1) {
		//	format = GL_RED;
		//} else if (num_components == 3) {
		//	format = GL_RGB;
		//} else if (num_components == 4) {
		//	format = GL_RGBA;
		//}
		switch (num_components) {
			case 1: format = GL_RED; break;
			case 3: format = GL_RGB; break;
			case 4: format = GL_RGBA; break;
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
	this->bind();
	glTexImage2D(
		m_target,
		0,
		internalFormat,
		width,
		height,
		0,
		format,
		type,
		data
	);
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_filterMin);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_filterMax);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_S, m_wrapS);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_T, m_wrapT);
	if (m_mipmapping) {
		glGenerateMipmap(m_target);
	}
	this->unbind();
}

void GLTexture::bind(int unit) const {
	if (unit >= 0) {
		glActiveTexture(GL_TEXTURE0 + unit);
	}
	glBindTexture(m_target, m_ID);
}
void GLTexture::unbind() const {
	glBindTexture(m_target, 0);
}

auto GLTexture::resize(uint32_t width, uint32_t height, uint32_t depth) {

	this->bind();
	switch (m_target) {
		case GL_TEXTURE_1D: 
			glTexImage1D(GL_TEXTURE_1D, 0, m_internal_format, width, 0, m_format, m_type, 0);
			break;
		case GL_TEXTURE_2D:
			assert(height > 0);
			glTexImage2D(GL_TEXTURE_2D, 0, m_internal_format, width, height, 0, m_format, m_type, 0);
			break;
		case GL_TEXTURE_3D: 
			assert(height > 0 && depth > 0);
			glTexImage3D(GL_TEXTURE_3D, 0, m_internal_format, width, height, depth, 0, m_format, m_type, 0);
			break;
		default: assert(false);
	}
}
