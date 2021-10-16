#include "pch.h"
#include "font_manager.h"
#include <map>
#include <glad/glad.h>
#include "math/vec.h"
#include "freetype/include/ft2build.h"

#include "graphics/opengl/opengl_texture.h"

#include FT_FREETYPE_H
#include FT_STROKER_H

namespace JadeFrame {
auto FontManager::init() -> void {
	struct Character {
		GLuint TextureID;  // ID handle of the glyph texture
		v2 Size;       // Size of glyph
		v2 Bearing;    // Offset from baseline to left/top of glyph
		GLuint Advance;    // Offset to advance to next glyph
	};
	std::map<GLchar, Character> character_map;

	FT_Error error_code;
	FT_Library library;
	error_code = FT_Init_FreeType(&library);
	if (error_code != 0) {
		Logger::log("problem with FT_Init_FreeType");
	}

	FT_Face face;
	error_code = FT_New_Face(library, "C:/Windows/Fonts/Arial.ttf", 0, &face);
	if (error_code != 0) {
		Logger::log("problem with FT_New_Face");
	}

	error_code = FT_Set_Pixel_Sizes(face, 0, 48);
	if (error_code != 0) {
		Logger::log("problem with FT_Set_Pixel_Sizes");
	}


	auto c = 'X';
	error_code = FT_Load_Char(face, c, FT_LOAD_RENDER);
	if (error_code != 0) {
		Logger::log("problem with FT_Load_Char");
	}
	OGLW_Texture<GL_TEXTURE_2D> tex;
	tex.bind(0);
	tex.set_texture_image_2D(
		0,
		GL_RED,
		face->glyph->bitmap.width,
		face->glyph->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		face->glyph->bitmap.buffer
	);

	tex.set_texture_parameters(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	tex.set_texture_parameters(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	tex.set_texture_parameters(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	tex.set_texture_parameters(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

auto FontManager::init2() -> void {
	struct Character {
		GLuint TextureID;  // ID handle of the glyph texture
		v2 Size;       // Size of glyph
		v2 Bearing;    // Offset from baseline to left/top of glyph
		GLuint Advance;    // Offset to advance to next glyph
	};
	std::map<GLchar, Character> character_map;

	FT_Error error_code;
	FT_Library library;
	error_code = FT_Init_FreeType(&library);
	if (error_code != 0) {
		Logger::log("problem with FT_Init_FreeType");
	}

	FT_Face face;
	error_code = FT_New_Face(library, "C:/Windows/Fonts/Arial.ttf", 0, &face);
	if (error_code != 0) {
		Logger::log("problem with FT_New_Face");
	}

	error_code = FT_Set_Pixel_Sizes(face, 0, 48);
	if (error_code != 0) {
		Logger::log("problem with FT_Set_Pixel_Sizes");
	}


	auto c = 'X';
	error_code = FT_Load_Char(face, c, FT_LOAD_RENDER);
	if (error_code != 0) {
		Logger::log("problem with FT_Load_Char");
	}
	OpenGL_Texture tex(
		face->glyph->bitmap.buffer,
		{ face->glyph->bitmap.width, face->glyph->bitmap.rows },
		GL_RED,
		GL_RED,
		GL_UNSIGNED_BYTE
	);
}
}