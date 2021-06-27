#include "opengl_texture.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <iostream>
#include "JadeFrame/defines.h"

namespace JadeFrame {

struct STBIImage {
	STBIImage(const std::string& path) {
		// flip textures on their y coordinate while loading
		stbi_set_flip_vertically_on_load(true);
		//i32 width, height, num_components;
		data = stbi_load(path.c_str(), &width, &height, &num_components, 0);
	}
	~STBIImage() {
		stbi_image_free(data);
	}
	i32 width, height, num_components;
	unsigned char* data;
};

auto OpenGL_TextureLoader::load(const std::string& path, GLenum target, const GLenum internal_format, bool is_srgb) -> OpenGL_Texture {
	OpenGL_Texture texture;
	texture.m_target = target;
	//texture.m_internal_format = internal_format;
	if (internal_format == GL_RGB || internal_format == GL_SRGB) {
		texture.m_internal_format = is_srgb ? GL_SRGB : GL_RGB;
	}
	if (internal_format == GL_RGBA || internal_format == GL_SRGB_ALPHA) {
		texture.m_internal_format = is_srgb ? GL_SRGB_ALPHA : GL_RGBA;
	}

	STBIImage image(path);
	if (image.data) {
		GLenum format = static_cast<GLenum>(-1);
		switch (image.num_components) {
			case 1: format = GL_RED; break;
			case 3: format = GL_RGB; break;
			case 4: format = GL_RGBA; break;
		}

		if (target == GL_TEXTURE_2D) {
			texture.generate(image.width, image.height, texture.m_internal_format, format, GL_UNSIGNED_BYTE, image.data);
		} else {
			__debugbreak();
		}
	} else {
		std::cout << "Texture failed to load at path: " + path << std::endl;
		return texture;
	}
	texture.m_width = image.width;
	texture.m_height = image.height;

	return texture;
}



void OpenGL_Texture::generate(u32 width, u32 height, GLenum internal_format, GLenum format, GLenum type, void* data) {
	//assert(Target == GL_TEXTURE_2D);
	tex.bind(m_target);
	tex.set_texture_image_2D(
		m_target,
		0,
		internal_format,
		width,
		height,
		0,
		format,
		type,
		data
	);

	tex.set_texture_parameters(m_target, GL_TEXTURE_MIN_FILTER, m_filterMin);
	tex.set_texture_parameters(m_target, GL_TEXTURE_MAG_FILTER, m_filterMax);
	tex.set_texture_parameters(m_target, GL_TEXTURE_WRAP_S, m_wrapS);
	tex.set_texture_parameters(m_target, GL_TEXTURE_WRAP_T, m_wrapT);
	if (m_mipmapping) {
		tex.generate_mipmap(m_target);
	}
	tex.unbind(m_target);

	m_width = width;
	m_height = height;
	m_depth = 0;
	m_internal_format = internal_format;
	m_format = format;
	m_type = type;
}
auto OpenGL_Texture::resize(u32 width, u32 height, u32 depth)-> void {

	tex.bind(m_target);
	switch (m_target) {
		case GL_TEXTURE_1D:
			assert(width > 0);
			tex.set_texture_image_1D(GL_TEXTURE_1D, 0, m_internal_format, width, 0, m_format, m_type, 0);
			break;
		case GL_TEXTURE_2D:
			assert(width > 0 && height > 0);
			tex.set_texture_image_2D(GL_TEXTURE_2D, 0, m_internal_format, width, height, 0, m_format, m_type, 0);
			break;
		case GL_TEXTURE_3D:
			assert(width > 0 && height > 0 && depth > 0);
			tex.set_texture_image_3D(GL_TEXTURE_3D, 0, m_internal_format, width, height, depth, 0, m_format, m_type, 0);
			break;
		default: assert(false);
	}
}

auto OpenGL_Texture::bind() const -> void {
	tex.bind(m_target);
}

}