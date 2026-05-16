#pragma once
#include "graphics/graphics_shared.h"

/*
    TODO: For now glfw effective only works for windows

*/

namespace JadeFrame {
class Window;
struct WindowEvent;

class GUI {
public:
    ~GUI();

    auto init(Window* window, GRAPHICS_API api) -> void;
    auto new_frame() -> void;
    auto render() -> void;
    auto destroy() -> void;
    auto on_window_event(const WindowEvent& event) -> void;

public:
    bool         m_is_initialized = false;
    bool         m_has_context = false;
    bool         m_platform_backend_initialized = false;
    bool         m_renderer_backend_initialized = false;
    GRAPHICS_API m_graphics_api = GRAPHICS_API::OPENGL;
    Window*      m_window = nullptr;
    u64          m_window_event_callback_id = 0;
};
} // namespace JadeFrame
