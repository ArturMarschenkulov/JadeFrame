#pragma once

namespace JadeFrame {
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

struct WindowEvent {
    enum class TYPE {
        KEY,
        BUTTON,
        MOUSE,
    };
    TYPE type;

    union {
        KeyEvent    key_event;
        ButtonEvent button_event;
        MouseEvent  mouse_event;
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

    auto is_empty() -> bool { return m_queue.empty(); }

    auto clear() -> void { m_queue.clear(); }

public:
    std::deque<WindowEvent> m_queue = {};
};
} // namespace JadeFrame