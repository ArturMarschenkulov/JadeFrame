#pragma once
#include <glad/glad.h>

#include <string>

class GLTexture {
public:
	GLuint m_ID;
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



	static auto load(const std::string& path, GLenum target, GLenum internalFormat, bool srgb = false) -> GLTexture;

	auto generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, void* data) -> void;

	auto bind(int unit = -1) -> void;
	auto unbind() -> void;
};
