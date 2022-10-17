#pragma once
#include "graphics/graphics_shared.h"

/*
    TODO: For now glfw effective only works for windows
    
*/
struct HWND__;
typedef HWND__* HWND;

namespace JadeFrame {
class GUI {
public:
    auto init(HWND window, GRAPHICS_API api) -> void;
    auto new_frame() -> void;
    auto render() -> void;
    auto destroy() -> void;
};
// auto GUI_init(HWND window) -> void;
// auto GUI_new_frame() -> void;
// auto GUI_render() -> void;
// auto GUI_destroy() -> void;
} // namespace JadeFrame