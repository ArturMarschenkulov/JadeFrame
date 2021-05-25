#include "opengl_context.h"
#include "opengl_debug.h"
#include <iostream>
#if _WIN32
#include "opengl_windows.h"
#endif


OpenGL_Context::OpenGL_Context(HWND hWnd) {

	static bool is_wgl_loaded = false;
	if (is_wgl_loaded == false) {
		is_wgl_loaded = wgl_load_0();
	}

	HDC device_context = GetDC(hWnd);
	if(device_context == NULL) {
		std::cout << "GetDC(hWnd) failed! " << ::GetLastError() << std::endl;
		__debugbreak();
	}
	wgl_set_pixel_format(device_context);
	HGLRC render_context = wgl_create_render_context(device_context);

	m_window_handle = hWnd;
	m_device_context = device_context;
	m_render_context = render_context;

	if (gladLoadGL() != 1) {
		std::cout << "gladLoadGL() failed." << std::endl;
	}

	set_debug_mode(true);
	gl_cache.set_default();


	vendor = reinterpret_cast<char const*>(glGetString(GL_VENDOR));
	renderer = reinterpret_cast<char const*>(glGetString(GL_RENDERER));
	version = reinterpret_cast<char const*>(glGetString(GL_VERSION));
	shading_language_version = reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	glGetIntegerv(GL_MAJOR_VERSION, &major_version);
	glGetIntegerv(GL_MINOR_VERSION, &minor_version);
	glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
	for (u32 i = 0; i < num_extensions; i++) {
		extentenions.push_back(reinterpret_cast<char const*>(glGetStringi(GL_EXTENSIONS, i)));
	}

	//glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
	//glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
	//glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
	//glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
}

OpenGL_Context::~OpenGL_Context() {
}

auto GL_Cache::set_default() -> void {
	set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
	set_depth_test(true);
	set_clear_bitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	set_blending(true);
	set_polygon_mode(POLYGON_FACE::FRONT_AND_BACK, POLYGON_MODE::FILL);
	set_face_culling(false, GL_BACK);
}

auto GL_Cache::set_blending(bool enable, BLENDING_FACTOR sfactor, BLENDING_FACTOR dfactor) -> void {
	if (blending != enable) {
		blending = enable;
		if (enable) {
			glEnable(GL_BLEND);
			glBlendFunc(sfactor, dfactor);
		} else {
			glDisable(GL_BLEND);
		}
	}
}

auto GL_Cache::set_clear_color(const Color& color) -> void {
	if (clear_color != color) {
		clear_color = color;
		glClearColor(color.r, color.g, color.b, color.a);
	}
}

auto GL_Cache::set_polygon_mode(POLYGON_FACE face, POLYGON_MODE mode) -> void {
	if ((polygon_mode.first != face) || (polygon_mode.second != mode)) {
		polygon_mode = { face, mode };

		glPolygonMode((GLenum)face, (GLenum)mode);
	}
}

auto GL_Cache::set_clear_bitfield(const GLbitfield& bitfield) -> void {
	clear_bitfield = bitfield;
}

auto GL_Cache::add_clear_bitfield(const GLbitfield& bitfield) -> void {
	clear_bitfield |= (1 << bitfield);
}

auto GL_Cache::remove_clear_bitfield(const GLbitfield& bitfield) -> void {
	clear_bitfield &= ~(1 << bitfield);
}

auto GL_Cache::set_depth_test(bool enable) -> void {
	if (depth_test != enable) {
		depth_test = enable;
		if (enable) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
	}
}

auto GL_Cache::set_face_culling(bool enable, GLenum mode) -> void {
	if (is_face_culling != enable) {
		is_face_culling = enable;
		if (enable) {
			glEnable(GL_CULL_FACE);
			glCullFace(mode);
		} else {
			glDisable(GL_CULL_FACE);
		}
	}
}
