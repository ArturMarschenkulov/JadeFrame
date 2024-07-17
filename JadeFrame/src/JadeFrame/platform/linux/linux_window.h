#pragma once

#include "../window.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace JadeFrame {

class X11_NativeWindow : public NativeWindow {
public:
    X11_NativeWindow(const X11_NativeWindow&) = delete;
    auto operator=(const X11_NativeWindow&) -> X11_NativeWindow& = delete;
    X11_NativeWindow(X11_NativeWindow&&) = delete;
    auto operator=(X11_NativeWindow&&) -> X11_NativeWindow& = delete;

    X11_NativeWindow() = default;
    explicit X11_NativeWindow(const Window::Desc& desc);
    ~X11_NativeWindow() override;

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