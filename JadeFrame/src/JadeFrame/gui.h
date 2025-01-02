#pragma once
#include "graphics/graphics_shared.h"

/*
    TODO: For now glfw effective only works for windows

*/

namespace JadeFrame {
class Window;

class GUI {
public:
    auto init(Window* window, GRAPHICS_API api) -> void;
    auto new_frame() -> void;
    auto render() -> void;
    auto destroy() -> void;

public:
    bool         m_is_initialized = false;
    GRAPHICS_API m_graphics_api = GRAPHICS_API::OPENGL;
};
} // namespace JadeFrame