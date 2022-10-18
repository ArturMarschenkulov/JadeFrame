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
} // namespace JadeFrame