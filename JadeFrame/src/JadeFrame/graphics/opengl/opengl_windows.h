#pragma once
#ifdef _WIN32
#include "Windows.h"


/*
        NOTE: Make sure this header file is only only included in "opengl_context.h".
*/

namespace JadeFrame {
namespace opengl {
namespace win32 {
auto init_device_context(const HWND& window) -> HDC;
auto init_render_context(HDC device_context) -> HGLRC;
auto load() -> bool;
auto swap_interval(i32) -> void;
auto set_pixel_format(const HDC& device_context) -> void;
auto create_render_context(HDC device_context) -> HGLRC;
} // namespace win32
} // namespace opengl
} // namespace JadeFrame

#endif