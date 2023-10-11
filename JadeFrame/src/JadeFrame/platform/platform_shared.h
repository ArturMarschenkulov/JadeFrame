#pragma once
#include "JadeFrame/prelude.h"
#include "JadeFrame/math/vec.h"
#include <string>

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

    MAX
};

// TODO: Think of a better way to abstract everything. Right now CRTP seems to be enough.

class IWindow {
public:
    enum class WINDOW_STATE {
        WINDOWED,
        MINIMIZED,
        MAXIMIZED,
    };

    struct Desc {
        std::string title;
        v2u32       size;
        v2u32       position; // NOTE: -1 means randomly chosen by OS
        // bool is_vsync;
        WINDOW_STATE window_state = WINDOW_STATE::WINDOWED;
        bool         visable = true;
        bool         accept_drop_files = false;
    };

    virtual auto handle_events(bool& running)
        -> void = 0; // TODO: This is hacky. Fix it later
    virtual auto set_title(const std::string& title) -> void = 0;
    virtual auto get_title() const -> std::string = 0;
    virtual auto get_size() const -> const v2u32& = 0;
    virtual auto get_window_state() const -> WINDOW_STATE = 0;
    virtual auto get() const -> void* = 0;
};

class ISystemManager {
public:
    virtual auto initialize() -> void = 0;
    virtual auto log() const -> void = 0;

    virtual auto request_window(IWindow::Desc desc) -> IWindow* = 0;

    virtual auto get_time() const -> f64 = 0;
    virtual auto calc_elapsed() -> f64 = 0;
    virtual auto frame_control(f64 delta_time) -> void = 0;
    virtual auto set_target_FPS(f64 FPS) -> void = 0;
};

class IPlatform {
public:
    // platform stuff

    // input stuff

    // time stuff
    virtual auto get_time() const -> f64 = 0;
    virtual auto calc_elapsed() -> f64 = 0;
    virtual auto frame_control(f64 delta_time) -> void = 0;
    virtual auto set_FPS(f64 FPS) -> void = 0;
};

class SystemManager2 {
public:
    // This is the implementation
    ISystemManager* m_impl = nullptr;
};

class IInputManager {
public:
    virtual auto handle_input() -> void = 0;
    // static auto is_key_down(const KEY key) -> bool;
    // static auto is_key_up(const KEY key) -> bool;
    // static auto is_key_pressed(const KEY key) -> bool;
    // static auto is_key_released(const KEY key) -> bool;
};
} // namespace JadeFrame
