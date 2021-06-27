#pragma once
#include "Windows.h"

/*
	NOTE: Make sure this header file is only only included in "opengl_context.h".
*/

namespace JadeFrame {
auto wgl_load() -> bool;
auto wgl_swap_interval(i32) -> void;
auto wgl_set_pixel_format(const HDC& device_context) -> void;
auto wgl_create_render_context(HDC device_context)->HGLRC;
}