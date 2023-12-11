#pragma once
#include "JadeFrame/prelude.h"
#include "JadeFrame/math/vec.h"
#include <string>
#include <memory>

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

class NativeWindow;

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

    IWindow() = default;

    IWindow(const IWindow::Desc& desc);

    auto handle_events(bool& running) -> void;
    auto get_window_state() const -> WINDOW_STATE;
    auto set_title(const std::string& title) -> void;
    auto get_title() const -> std::string;
    auto get_size() const -> const v2u32&;

public:
    std::unique_ptr<NativeWindow> m_native_window;
};

// This class represents an interface for a native window, that is a win32, x11, wayland
// etc window. The purpose of this class is to abstract the platform specific window
// creation and event handling.
class NativeWindow {
public:

public:
    NativeWindow() = default;
    NativeWindow(const NativeWindow&) = delete;
    auto operator=(const NativeWindow&) -> NativeWindow& = delete;
    NativeWindow(NativeWindow&&) = delete;
    auto operator=(NativeWindow&&) -> NativeWindow& = delete;
    virtual ~NativeWindow() = default;

public:
    virtual auto handle_events(bool& running) -> void = 0;
    virtual auto set_title(const std::string& title) -> void = 0;
    virtual auto get_title() const -> std::string = 0;
    virtual auto get_size() const -> const v2u32& = 0;
    virtual auto get_window_state() const -> IWindow::WINDOW_STATE = 0;
};

template<typename T>
class ISystemManager {
public:
    virtual ~ISystemManager() = default;
    virtual auto initialize() -> void = 0;
    virtual auto log() const -> void = 0;

    virtual auto request_window(IWindow::Desc desc) -> IWindow* = 0;

    // time management
    auto get_time() const -> f64 { return static_cast<const T*>(this)->get_time(); }

    auto calc_elapsed() -> f64 { return static_cast<T*>(this)->calc_elapsed(); }

    auto frame_control(f64 delta_time) -> void {
        return static_cast<T*>(this)->frame_control(delta_time);
    }

    auto set_target_FPS(f64 FPS) -> void {
        return static_cast<T*>(this)->set_target_FPS(FPS);
    }
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

template<typename T>
class IInputManager {
public:
    virtual auto handle_input() -> void { static_cast<T*>(this)->handle_input(); }

    static auto is_key_down(const KEY key) -> bool {
        return IInputManager<T>::is_key_down(key);
    }

    static auto is_key_up(const KEY key) -> bool {
        return IInputManager<T>::is_key_up(key);
    }

    static auto is_key_pressed(const KEY key) -> bool {
        return IInputManager<T>::is_key_pressed(key);
    }

    static auto is_key_released(const KEY key) -> bool {
        return IInputManager<T>::is_key_released(key);
    }
};
} // namespace JadeFrame
