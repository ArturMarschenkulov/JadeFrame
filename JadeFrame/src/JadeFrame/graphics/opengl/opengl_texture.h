#pragma once
#include "JadeFrame/defines.h"
#include <glad/glad.h>
#include "opengl_object.h"

#include <string>

namespace JadeFrame {

class OpenGL_Texture;
class OpenGL_Texture {
public:
	auto init(u32 width, u32 height, GLenum internalFormat, GLenum format, GLenum type, void* data) -> void;
	auto resize(u32 width, u32 height, u32 depth) -> void;
	auto bind() const -> void;
	auto unbind() const -> void;
	GL_Texture<GL_TEXTURE_2D> m_texture;

	GLenum m_internal_format = GL_RGBA8;
	GLenum m_format = GL_RGBA;
	GLenum m_type = GL_UNSIGNED_BYTE;

	GLuint m_width = 0;
	GLuint m_height = 0;
};

}