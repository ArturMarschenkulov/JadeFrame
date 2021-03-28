#include "FontManager.h"
#include <iostream>
#include <map>
#include <glad/glad.h>
#include "math/Vec2.h"
#include "freetype/include/ft2build.h"

#include "graphics/opengl/GLTexture.h"

#include FT_FREETYPE_H
#include FT_STROKER_H
auto FontManager::init() -> void {
	struct Character {
		GLuint TextureID;  // ID handle of the glyph texture
		Vec2 Size;       // Size of glyph
		Vec2 Bearing;    // Offset from baseline to left/top of glyph
		GLuint Advance;    // Offset to advance to next glyph
	};
	std::map<GLchar, Character> character_map;

	FT_Error error_code;
	FT_Library library;
	error_code = FT_Init_FreeType(&library);
	if (error_code != 0) {
		std::cout << "problem with FT_Init_FreeType" << std::endl;
	}

	FT_Face face;
	error_code = FT_New_Face(library, "C:/Windows/Fonts/Arial.ttf", 0, &face);
	if (error_code != 0) {
		std::cout << "problem with FT_New_Face" << std::endl;
	}

	error_code = FT_Set_Pixel_Sizes(face, 0, 48);
	if (error_code != 0) {
		std::cout << "problem with FT_Set_Pixel_Sizes" << std::endl;
	}


	auto c = 'X';
	error_code = FT_Load_Char(face, c, FT_LOAD_RENDER);
	if (error_code != 0) {
		std::cout << "problem with FT_Load_Char" << std::endl;
	}
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		face->glyph->bitmap.width,
		face->glyph->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		face->glyph->bitmap.buffer
	);
	// Set texture options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Now store character for later use
	//Character character = {
	//	texture,
	//	Vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
	//	Vec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
	//	face->glyph->advance.x
	//};
	//character_map.insert(std::pair<GLchar, Character>(c, character));

}

auto FontManager::init2() -> void {
	struct Character {
		GLuint TextureID;  // ID handle of the glyph texture
		Vec2 Size;       // Size of glyph
		Vec2 Bearing;    // Offset from baseline to left/top of glyph
		GLuint Advance;    // Offset to advance to next glyph
	};
	std::map<GLchar, Character> character_map;

	FT_Error error_code;
	FT_Library library;
	error_code = FT_Init_FreeType(&library);
	if (error_code != 0) {
		std::cout << "problem with FT_Init_FreeType" << std::endl;
	}

	FT_Face face;
	error_code = FT_New_Face(library, "C:/Windows/Fonts/Arial.ttf", 0, &face);
	if (error_code != 0) {
		std::cout << "problem with FT_New_Face" << std::endl;
	}

	error_code = FT_Set_Pixel_Sizes(face, 0, 48);
	if (error_code != 0) {
		std::cout << "problem with FT_Set_Pixel_Sizes" << std::endl;
	}


	auto c = 'X';
	error_code = FT_Load_Char(face, c, FT_LOAD_RENDER);
	if (error_code != 0) {
		std::cout << "problem with FT_Load_Char" << std::endl;
	}
	GLTexture tex;
	tex.generate(
		face->glyph->bitmap.width, 
		face->glyph->bitmap.rows, 
		GL_RED, 
		GL_RED, 
		GL_UNSIGNED_BYTE, 
		face->glyph->bitmap.buffer
	);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		face->glyph->bitmap.width,
		face->glyph->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		face->glyph->bitmap.buffer
	);
	// Set texture options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}