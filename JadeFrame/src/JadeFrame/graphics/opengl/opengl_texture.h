#pragma once
#include "JadeFrame/defines.h"
#include <glad/glad.h>
#include "opengl_wrapper.h"

#include <string>

namespace JadeFrame {

class OpenGL_Texture;
class OpenGL_Texture {
public:
	OpenGL_Texture(u32 width, u32 height, GLenum internal_format, GLenum format, GLenum type, void* data);
	auto resize(u32 width, u32 height, u32 depth) -> void;
	auto bind() const -> void;
	auto unbind() const -> void;

public:
	OGLW_Texture<GL_TEXTURE_2D> m_texture;

	const GLenum m_internal_format;
	const GLenum m_format;
	const GLenum m_type;

	const GLuint m_width;
	const GLuint m_height;
};

}