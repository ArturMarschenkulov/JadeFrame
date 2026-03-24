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

    X11_NativeWindow() = delete;
    static auto create(const Window::Desc& desc) -> X11_NativeWindow;

    ~X11_NativeWindow() override;

    auto handle_events(bool& should_close) -> void override;

    auto set_title(const std::string& title) -> void override {
        m_title = title;
        if ((m_display != nullptr) && (m_window != 0U)) {
            XStoreName(m_display, m_window, m_title.c_str());
            XFlush(m_display);
        }
    }

    [[nodiscard]] auto get_title() const -> std::string override { return m_title; }

    [[nodiscard]] auto get_size() const -> const v2u32& override { return m_size; }

public:
    ::Display*     m_display = nullptr;
    ::XVisualInfo* m_visual_info = nullptr;
    ::Window       m_window = 0;
    v2u32          m_size;
    std::string    m_title;
    Atom           m_wm_delete;
    ::XIM          m_xim = nullptr;
    ::XIC          m_xic = nullptr;
};
} // namespace JadeFrame