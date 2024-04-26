#pragma once

#include "../platform_shared.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace JadeFrame {

class Linux_Window : public NativeWindow {
public:
    Linux_Window(const Linux_Window&) = delete;
    auto operator=(const Linux_Window&) -> Linux_Window& = delete;
    Linux_Window(Linux_Window&&) = delete;
    auto operator=(Linux_Window&&) -> Linux_Window& = delete;

    Linux_Window() = default;
    explicit Linux_Window(const IWindow::Desc& desc);
    ~Linux_Window() override;

    auto handle_events(bool&) -> void override;

    auto set_title(const std::string&) -> void override {}

    [[nodiscard]] auto get_title() const -> std::string override { return {}; }

    [[nodiscard]] auto get_size() const -> const v2u32& override { return m_size; }

public:
    ::Display*     m_display;
    ::XVisualInfo* m_visual_info;
    ::Window       m_window;
    v2u32          m_size;
};
} // namespace JadeFrame