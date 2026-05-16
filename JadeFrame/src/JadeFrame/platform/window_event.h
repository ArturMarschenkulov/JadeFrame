#pragma once
#include <deque>
#include "JadeFrame/types.h"

namespace JadeFrame {
enum class INPUT_STATE {
    RELEASED,
    PRESSED
};

enum class BUTTON : u8 {
    LEFT,
    RIGHT,
    MIDDLE,
    X1,
    X2,
    MAX
};
enum class KEY : u8 {
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
    // Denotes the number of keys and an invalid/unknown key
    MAX
};

struct KeyEvent {
    INPUT_STATE type;
    KEY         key;
};

struct ButtonEvent {
    INPUT_STATE type;
    BUTTON      button;
};

struct MouseEvent {
    i32 m_x;
    i32 m_y;
};

struct WindowResizeEvent {
    u32 width;
    u32 height;
};

struct WindowEvent {
    enum class TYPE : u8 {
        KEY,
        BUTTON,
        MOUSE,
        RESIZE,
        CLOSE,
    };

    static auto make_button_event(BUTTON button, INPUT_STATE state) -> WindowEvent {
        ButtonEvent button_event = {
            .type = state,
            .button = button,
        };
        WindowEvent we = {
            .type = WindowEvent::TYPE::BUTTON,
            .button_event = button_event,
        };
        return we;
    }

    static auto make_key_event(KEY key, INPUT_STATE state) -> WindowEvent {
        KeyEvent key_event = {
            .type = state,
            .key = key,
        };
        WindowEvent we = {
            .type = WindowEvent::TYPE::KEY,
            .key_event = key_event,
        };
        return we;
    }

    static auto make_mouse_event(i32 x, i32 y) -> WindowEvent {
        MouseEvent mouse_event = {
            .m_x = x,
            .m_y = y,
        };
        WindowEvent we = {
            .type = WindowEvent::TYPE::MOUSE,
            .mouse_event = mouse_event,
        };
        return we;
    }

    static auto make_resize_event(u32 width, u32 height) -> WindowEvent {
        WindowResizeEvent resize_event = {
            .width = width,
            .height = height,
        };
        WindowEvent we = {
            .type = WindowEvent::TYPE::RESIZE,
            .resize_event = resize_event,
        };
        return we;
    }

    static auto make_close_event() -> WindowEvent {
        WindowEvent we = {
            .type = WindowEvent::TYPE::CLOSE,
            .key_event = {},
        };
        return we;
    }

    TYPE type;

    union {
        KeyEvent          key_event;
        ButtonEvent       button_event;
        MouseEvent        mouse_event;
        WindowResizeEvent resize_event;
    };
};

class WindowEventQueue {
public:
    void push(const WindowEvent& event) { m_queue.push_back(event); }

    auto pop() -> WindowEvent {
        WindowEvent event = m_queue.front();
        m_queue.pop_front();
        return event;
    }

    [[nodiscard]] auto is_empty() const -> bool { return m_queue.empty(); }

    auto clear() -> void { m_queue.clear(); }

public:
    std::deque<WindowEvent> m_queue;
};
} // namespace JadeFrame
