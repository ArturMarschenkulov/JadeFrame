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
enum class INPUT_STATE {
    RELEASED,
    PRESSED
};

enum class BUTTON {
    LEFT,
    RIGHT,
    MIDDLE,
    X1,
    X2,
    MAX
};
enum class KEY {
    SPACE,
    ESCAPE,
    ENTER,
    TAB,
    BACKSPACE,
    INSERT,
    DELET,
    RIGHT,
    LEFT,
    DOWN,
    UP,
    PAGE_UP,
    PAGE_DOWN,
    HOME,
    END,
    CAPS_LOCK,
    SCROLL_LOCK,
    NUM_LOCK,
    PRINT_SCREEN,
    PAUSE,

    // Extra. Names may be changed
    OEM_1,
    OEM_PLUS,
    OEM_COMMA,
    OEM_MINUS,
    OEM_PERIOD,
    OEM_2,
    OEM_3,
    OEM_4,
    OEM_5,
    OEM_6,
    OEM_7,
    OEM_8,
    OEM_AX,
    OEM_102,
    ICO_HELP,
    ICO_00,
    // The F keys
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    LEFT_SHIFT,
    LEFT_CONTROL,
    LEFT_ALT,
    RIGHT_SHIFT,
    RIGHT_CONTROL,
    RIGHT_ALT,
    GRAVE,
    SLASH,
    BACKSLASH,

    LEFT_SUPER, // Windows key

    // The number keys
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,

    // The letter keys
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    MAX
};
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