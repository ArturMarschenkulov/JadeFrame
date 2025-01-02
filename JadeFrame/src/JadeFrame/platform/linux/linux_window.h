#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "../window.h"

namespace JadeFrame {

class X11_NativeWindow : public NativeWindow {
private:
    explicit X11_NativeWindow(const Window::Desc& desc);

public:
    X11_NativeWindow(const X11_NativeWindow&) = delete;
    auto operator=(const X11_NativeWindow&) -> X11_NativeWindow& = delete;
    X11_NativeWindow(X11_NativeWindow&& other) noexcept;
    auto operator=(X11_NativeWindow&& other) noexcept -> X11_NativeWindow&;

    X11_NativeWindow() = default;
    static auto create(const Window::Desc& desc) -> X11_NativeWindow;

    ~X11_NativeWindow() override;

    auto handle_events(bool&) -> void override;

    auto set_title(const std::string&) -> void override {}

    [[nodiscard]] auto get_title() const -> std::string override { return {}; }

    [[nodiscard]] auto get_size() const -> const v2u32& override { return m_size; }

public:
    ::Display*     m_display = nullptr;
    ::XVisualInfo* m_visual_info = nullptr;
    ::Window       m_window = 0;
    v2u32          m_size = {};
};
} // namespace JadeFrame