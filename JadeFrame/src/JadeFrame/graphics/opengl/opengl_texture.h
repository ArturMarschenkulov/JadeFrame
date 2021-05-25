#pragma once
#include "JadeFrame/defines.h"
#include <glad/glad.h>
#include "opengl_object.h"

#include <string>


class OpenGL_Texture;
class OpenGL_TextureLoader {
public:
	static auto load(const std::string& path, GLenum target, GLenum internalFormat, bool srgb = false)->OpenGL_Texture;
};

class OpenGL_Texture {
public:
	auto generate(u32 width, u32 height, GLenum internalFormat, GLenum format, GLenum type, void* data) -> void;
	auto resize(u32 width, u32 height, u32 depth) -> void;
	auto bind() const -> void;
	GL_Texture tex;

	GLenum m_target = GL_TEXTURE_2D;
	GLenum m_internal_format = GL_RGBA;
	GLenum m_format = GL_RGBA;
	GLenum m_type = GL_UNSIGNED_BYTE;
	GLenum m_filterMin = GL_LINEAR_MIPMAP_LINEAR;
	GLenum m_filterMax = GL_LINEAR;
	GLenum m_wrapS = GL_REPEAT;
	GLenum m_wrapT = GL_REPEAT;
	GLenum m_wrapR = GL_REPEAT;
	bool m_mipmapping = true;
	GLuint m_width = 0;
	GLuint m_height = 0;
	GLuint m_depth = 0;

};