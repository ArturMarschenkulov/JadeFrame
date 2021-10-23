#include "pch.h"
#include "opengl_texture.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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


OpenGL_Texture::OpenGL_Texture(void* data, v2u32 size, GLenum internal_format, GLenum format, GLenum type)
	: m_size(size)
	, m_internal_format(internal_format)
	, m_format(format)
	, m_type(type) {

	m_texture.bind(0);

	GLenum filter_min = GL_LINEAR;
	GLenum filter_max = GL_LINEAR;// GL_NEAREST;
	GLenum wrap_s = GL_REPEAT;
	GLenum wrap_t = GL_REPEAT;

	m_texture.set_texture_parameters(GL_TEXTURE_WRAP_S, wrap_s);
	m_texture.set_texture_parameters(GL_TEXTURE_WRAP_T, wrap_t);
	m_texture.set_texture_parameters(GL_TEXTURE_MIN_FILTER, filter_min);
	m_texture.set_texture_parameters(GL_TEXTURE_MAG_FILTER, filter_max);

	m_texture.set_texture_image_2D(
		0,
		internal_format,
		size.width,
		size.height,
		0,
		format,
		type,
		data
	);
	//if (m_mipmapping) {
	m_texture.generate_mipmap();
	//}
	m_texture.unbind();
}
auto OpenGL_Texture::resize(u32 width, u32 height, u32 /*depth*/)-> void {

	m_texture.bind(0);
	assert(width > 0 && height > 0);
	m_texture.set_texture_image_2D(0, m_internal_format, width, height, 0, m_format, m_type, 0);

	//switch (m_target) {
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
	//}
}

auto OpenGL_Texture::bind() const -> void {
	m_texture.bind(0);
}
auto OpenGL_Texture::unbind() const -> void {
	m_texture.unbind();
}

struct OGLTexture {
	auto create(const std::string& path) {
		STBIImage image(path);
		if (image.data) {

			m_width = image.width;
			m_height = image.height;

			GLenum internal_format = 0;
			GLenum data_format = 0;

			switch (image.num_components) {
				case 3:
				{
					internal_format = GL_RGB8;
					data_format = GL_RGB;
				} break;
				case 4:
				{
					internal_format = GL_RGBA8;
					data_format = GL_RGBA;
				} break;
				default: __debugbreak();
			}


			m_internal_format = internal_format;
			m_data_format = data_format;


			const GLenum target = GL_TEXTURE_2D;
			const GLenum filter_min = GL_LINEAR;
			const GLenum filter_max = GL_NEAREST;
			const GLenum wrap_s = GL_REPEAT;
			const GLenum wrap_t = GL_REPEAT;
			m_texture.bind(0);

			m_texture.set_texture_parameters(GL_TEXTURE_MIN_FILTER, filter_min);
			m_texture.set_texture_parameters(GL_TEXTURE_MAG_FILTER, filter_max);
			m_texture.set_texture_parameters(GL_TEXTURE_WRAP_S, wrap_s);
			m_texture.set_texture_parameters(GL_TEXTURE_WRAP_T, wrap_t);
			m_texture.generate_mipmap();
			m_texture.unbind();
		}
	}

public:
	OGLW_Texture<GL_TEXTURE_2D> m_texture;
	u32 m_width;
	u32 m_height;
	GLenum m_internal_format;
	GLenum m_data_format;
};


}