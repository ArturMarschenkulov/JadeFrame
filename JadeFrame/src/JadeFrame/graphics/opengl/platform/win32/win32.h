
#pragma once
#ifdef _WIN32
#include "Windows.h"


/*
        NOTE: Make sure this header file is only only included in "opengl_context.h".
*/

namespace JadeFrame {
class IWindow;
namespace opengl {
namespace win32 {
auto init_render_context(HDC device_context) -> HGLRC;
auto load_wgl_funcs(HMODULE module) -> bool;
auto load_opengl_funcs(/*HDC device_context, HGLRC render_context*/) -> bool;
auto swap_interval(i32) -> void;
auto set_pixel_format(const HDC& device_context) -> void;
auto create_render_context(HDC device_context) -> HGLRC;
} // namespace win32
} // namespace opengl
} // namespace JadeFrame

#endif