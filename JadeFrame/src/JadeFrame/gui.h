#pragma once
#include "graphics/graphics_shared.h"

/*
    TODO: For now glfw effective only works for windows
    
*/

namespace JadeFrame {
class IWindow;
class GUI {
public:
    auto init(IWindow* window, GRAPHICS_API api) -> void;
    auto new_frame() -> void;
    auto render() -> void;
    auto destroy() -> void;
};
// auto GUI_init(HWND window) -> void;
// auto GUI_new_frame() -> void;
// auto GUI_render() -> void;
// auto GUI_destroy() -> void;
} // namespace JadeFrame