#pragma once

#include "JadeFrame/math/vec.h"
#include "../platform_shared.h"

#include <string>

struct HWND__;
typedef HWND__* HWND;



namespace JadeFrame {



/*
        TOOD: Consider whether this class should be RAII or not. Right now it is.
*/

namespace win32 {
class Window : public IWindow {
public:
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    auto operator=(const Window&) -> Window& = delete;
    auto operator=(Window&&) -> Window& = delete;

    Window() = default;
    Window(const Window::Desc& desc, ::HMODULE instance);
    ~Window();



    virtual auto handle_events(bool& running) -> void override;

    virtual auto set_title(const std::string& title) -> void override;
    virtual auto get_title() const -> std::string override;

    auto set_size(const v2u32& size) -> void;
    auto get_size() const -> const v2u32& override;

    auto set_position(const v2u32& position) -> void;
    auto get_position() const -> const v2u32&;

    auto         set_window_state(const WINDOW_STATE window_state) -> void;
    virtual auto get_window_state() const -> WINDOW_STATE override;

    auto query_client_size() const -> v2u64;

    virtual auto get() const -> void* override;



public:
    HWND    m_window_handle = nullptr;
    HMODULE m_instance_handle = nullptr;

public:
    std::string m_title;
    v2u32       m_size;
    v2u32       m_position;
    bool        has_focus = true;

    WINDOW_STATE m_window_state = WINDOW_STATE::MINIMIZED;
};
} // namespace win32

} // namespace JadeFrame