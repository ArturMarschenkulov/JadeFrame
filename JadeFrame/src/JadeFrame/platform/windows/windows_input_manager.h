#pragma once
#include "JadeFrame/prelude.h"
#include "JadeFrame/math/vec.h"
#include "../platform_shared.h"
#include <Windows.h>

#include <array>
#include <queue>

namespace JadeFrame {
namespace win32 {
struct EventMessage;

struct Event {};
struct KeyEvent : public Event {
    enum class TYPE {
        PRESSED,
        RELEASED
    };
    KeyEvent(const TYPE type, const u64 key_code)
        : type(type)
        , key_code(key_code) {}
    TYPE type;
    u64  key_code;
};
class InputManager : public IInputManager {
    friend class WinWindow;

public:
    virtual auto handle_input() -> void override;

public:
    // key part
    std::queue<KeyEvent> key_buffer;
    // static auto key_callback(i64 lParam, u64 wParam, u32 message) -> void;
    static auto key_callback(const EventMessage& window_message) -> void;
    static auto key_callback2(i64 lParam, u64 wParam, u32 message) -> void;
    static auto char_callback(const EventMessage& window_message) -> void;

    static auto is_key_down(const KEY key) -> bool;
    static auto is_key_up(const KEY key) -> bool;
    static auto is_key_pressed(const KEY key) -> bool;
    static auto is_key_released(const KEY key) -> bool;

    static std::array<INPUT_STATE, static_cast<u32>(KEY::MAX)> m_current_key_state;
    static std::array<INPUT_STATE, static_cast<u32>(KEY::MAX)> m_previous_key_state;

    static auto translate_key_code(const u64 key_code) -> KEY;
    static auto translate_button_code(const u64 button_code) -> BUTTON;

public:
    // mouse part
    static auto mouse_button_callback(const EventMessage& window_message) -> void;

    auto is_button_down(const BUTTON button) const -> bool;
    auto is_button_up(const BUTTON button) const -> bool;
    auto is_button_pressed(const BUTTON button) const -> bool;
    auto is_button_released(const BUTTON button) const -> bool;
    auto get_mouse_position() const -> v2;

    static std::array<INPUT_STATE, static_cast<u32>(BUTTON::MAX)> m_current_mouse_button_state;
    static std::array<INPUT_STATE, static_cast<u32>(BUTTON::MAX)> m_previous_mouse_button_state;
    static v2                                                     m_mouse_posiition;
};
} // namespace win32
#ifdef _WIN32
using InputManager = win32::InputManager;
#endif
} // namespace JadeFrame