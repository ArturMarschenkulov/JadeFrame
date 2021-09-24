#include "pch.h"
#include "material_handle.h"
#include "opengl/opengl_shader.h"
#include "opengl/opengl_texture.h"
#include "stb/stb_image.h"
#include <cassert>

namespace JadeFrame {

TextureHandle::TextureHandle(const std::string& path) {
	// flip textures on their y coordinate while loading
	stbi_set_flip_vertically_on_load(true);
	//i32 width, height, num_components;
	m_data = stbi_load(path.c_str(), &m_width, &m_height, &m_num_components, 4);
}

TextureHandle::~TextureHandle() {
	stbi_image_free(m_data);
}

auto TextureHandle::init() -> void {


	switch (m_api) {
		case GRAPHICS_API::OPENGL:
		{
			GLenum format;
			switch (m_num_components) {
				case 3:
				{
					format = GL_RGB;
				}break;
				case 4:
				{
					format = GL_RGBA;
				}break;
				default: assert(false);
			}
			OpenGL_Texture* texture = new OpenGL_Texture(m_width, m_height, format, format, GL_UNSIGNED_BYTE, m_data);
			m_handle = texture;


		}break;
		case GRAPHICS_API::VULKAN:
		{
		}break;
		default: __debugbreak();
	}
}

ShaderHandle::ShaderHandle(const DESC& desc) {
	m_code = desc.shading_code;
	m_vertex_format = desc.vertex_format;
}

auto ShaderHandle::init() -> void {

	if (m_code.m_vertex_shader != "" && m_code.m_fragment_shader != "") {
		switch (api) {
			case API::OPENGL:
			{
				OpenGL_Shader::DESC shader_desc;
				shader_desc.code = m_code;
				m_handle = new OpenGL_Shader(shader_desc);

			}break;
			case API::VULKAN:
			{
				//m_handle = new Vulkan_Shader({ vertex_shader_code, fragment_shader_code });
			}break;
		}
	} else {
		assert(!"There was no shader code");
	}
}

auto MaterialHandle::init() const -> void {

	m_shader_handle->init();

	if (m_texture_handle != nullptr) {
		m_texture_handle->init();
	}

}

}