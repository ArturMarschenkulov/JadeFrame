#include "opengl_context.h"
#include "opengl_debug.h"
#include <iostream>
#if _WIN32
#include "opengl_windows.h"
#endif


GLContext::GLContext(HWND hWnd, HDC& o_device_context, HGLRC& o_render_context) {
	static bool is_wgl_loaded = false;
	if (is_wgl_loaded == false) {
		is_wgl_loaded = wgl_load_0();
	}

	HDC device_context = GetDC(hWnd);
	wgl_set_pixel_format(device_context);
	HGLRC render_context = wgl_create_render_context(device_context);

	o_device_context = device_context;
	o_render_context = render_context;

	m_window_handle = hWnd;
	m_device_context = o_device_context;
	m_render_context = o_render_context;

	if (gladLoadGL() != 1) {
		std::cout << "gladLoadGL() failed." << std::endl;
	}

	set_debug_mode(true);
	gl_cache.set_default();
}

GLContext::~GLContext() {
}

auto GLCache::set_default() -> void {
	set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
	set_depth_test(true);
	set_clear_bitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	set_blending(true);
	set_polygon_mode(POLYGON_FACE::FRONT_AND_BACK, POLYGON_MODE::FILL);
}

auto GLCache::set_blending(bool enable, BLENDING_FACTOR sfactor, BLENDING_FACTOR dfactor) -> void {
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

auto GLCache::set_clear_color(const Color& color) -> void {
	if (clear_color != color) {
		clear_color = color;
		glClearColor(color.r, color.g, color.b, color.a);
	}
}

auto GLCache::set_polygon_mode(POLYGON_FACE face, POLYGON_MODE mode) -> void {
	if ((polygon_mode.first != face) || (polygon_mode.second != mode)) {
		polygon_mode = { face, mode };

		glPolygonMode((GLenum)face, (GLenum)mode);
	}
}

auto GLCache::set_clear_bitfield(const GLbitfield& bitfield) -> void {
	clear_bitfield = bitfield;
}

auto GLCache::add_clear_bitfield(const GLbitfield& bitfield) -> void {
	clear_bitfield |= (1 << bitfield);
}

auto GLCache::remove_clear_bitfield(const GLbitfield& bitfield) -> void {
	clear_bitfield &= ~(1 << bitfield);
}

auto GLCache::set_depth_test(bool enable) -> void {
	if (depth_test != enable) {
		depth_test = enable;
		if (enable) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
	}
}