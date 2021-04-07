#include "Windows.h"

auto wgl_load_0() -> bool;
auto wgl_set_pixel_format(const HDC& device_context) -> void;
auto wgl_create_render_context(HDC device_context) -> HGLRC;