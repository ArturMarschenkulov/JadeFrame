#pragma once

#include "../platform_shared.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace JadeFrame {

class Linux_Window : public IWindow {
public:
    Linux_Window(const Linux_Window&) = delete;
    Linux_Window(Linux_Window&&) = delete;
    auto operator=(const Linux_Window&) -> Linux_Window& = delete;
    auto operator=(Linux_Window&&) -> Linux_Window& = delete;

    Linux_Window() = default;
    Linux_Window(const Linux_Window::Desc& desc);
    ~Linux_Window();

    virtual auto handle_events(bool&) -> void override {}

    virtual auto set_title(const std::string&) -> void override {}

    virtual auto get_title() const -> std::string override { return {}; }

    virtual auto get_window_state() const -> WINDOW_STATE override { return {}; }

    virtual auto get_size() const -> const v2u32& override { return m_size; }

    virtual auto get() const -> void* override { return nullptr; }

public:
    ::Display*     m_display;
    ::XVisualInfo* m_visual_info;
    ::Window       m_window;
    v2u32          m_size;
};
#ifdef __linux__
using Window = Linux_Window;
#endif
} // namespace JadeFrame