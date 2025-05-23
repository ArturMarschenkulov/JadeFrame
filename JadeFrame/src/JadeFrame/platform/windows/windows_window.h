#pragma once

#include "../platform_shared.h"
#include "JadeFrame/math/vec.h"

#include <string>

struct HWND__;
typedef HWND__* HWND;

namespace JadeFrame {

namespace win32 {
class NativeWindow : public JadeFrame::NativeWindow {
public:
    NativeWindow(const NativeWindow&) = delete;
    NativeWindow(NativeWindow&&) = delete;
    auto operator=(const NativeWindow&) -> NativeWindow& = delete;
    auto operator=(NativeWindow&&) -> NativeWindow& = delete;

    NativeWindow() = default;
    NativeWindow(const JadeFrame::Window::Desc& desc);
    ~NativeWindow();

    auto handle_events(bool& running) -> void override;

    auto set_title(const std::string& title) -> void override;
    auto get_title() const -> std::string override;

    auto set_size(const v2u32& size) -> void;
    auto get_size() const -> const v2u32& override;

    auto set_position(const v2u32& position) -> void;
    auto get_position() const -> const v2u32&;

    auto set_window_state(const JadeFrame::Window::WINDOW_STATE window_state) -> void;
    auto get_window_state() const -> JadeFrame::Window::WINDOW_STATE;

    auto query_client_size() const -> v2u64;

public:
    HWND    m_window_handle = nullptr;
    HMODULE m_instance_handle = nullptr;

public:
    std::string m_title;
    v2u32       m_size;
    v2u32       m_position;
    bool        has_focus = true;

    JadeFrame::Window::WINDOW_STATE m_window_state =
        JadeFrame::Window::WINDOW_STATE::MINIMIZED;
};
} // namespace win32

} // namespace JadeFrame