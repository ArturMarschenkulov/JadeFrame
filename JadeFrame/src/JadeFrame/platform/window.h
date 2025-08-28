#pragma once
#include "JadeFrame/platform/window_event.h"
#include "JadeFrame/prelude.h"
#include "JadeFrame/math/vec.h"

namespace JadeFrame {
class NativeWindow;

class InputState {
public:
    [[nodiscard]] auto is_key_down(const KEY key) const -> bool;
    [[nodiscard]] auto is_key_up(const KEY key) const -> bool;
    [[nodiscard]] auto is_key_pressed(const KEY key) const -> bool;
    [[nodiscard]] auto is_key_released(const KEY key) const -> bool;

public:
    std::array<INPUT_STATE, static_cast<u32>(KEY::MAX)> m_curr_key_state = {};
    std::array<INPUT_STATE, static_cast<u32>(KEY::MAX)> m_prev_key_state = {};

public:
    [[nodiscard]] auto is_button_down(const BUTTON button) const -> bool;
    [[nodiscard]] auto is_button_up(const BUTTON button) const -> bool;
    [[nodiscard]] auto is_button_pressed(const BUTTON button) const -> bool;
    [[nodiscard]] auto is_button_released(const BUTTON button) const -> bool;
    [[nodiscard]] auto get_mouse_position() const -> v2;

public:
    std::array<INPUT_STATE, static_cast<u32>(BUTTON::MAX)> m_curr_button_state = {};
    std::array<INPUT_STATE, static_cast<u32>(BUTTON::MAX)> m_prev_button_state = {};
    v2                                                     m_mouse_pos;

public:
    auto update() -> void;
};

class Window {
public:
    enum class WINDOW_STATE: u8 {
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
        bool         visible = true;
        bool         accept_drop_files = false;
    };

    explicit Window(const Window::Desc& desc);
    Window() = default;
    Window(const Window&) = delete;
    auto operator=(const Window&) -> Window& = delete;
    Window(Window&& other) noexcept;
    auto operator=(Window&& other) noexcept -> Window&;

    auto               handle_events(bool& running) -> void;
    [[nodiscard]] auto get_window_state() const -> WINDOW_STATE;
    auto               set_title(const std::string& title) -> void;
    [[nodiscard]] auto get_title() const -> std::string;
    [[nodiscard]] auto get_size() const -> const v2u32&;

public:
    using EventCallback = std::function<void(const WindowEvent&)>;
    void                       add_event_callback(EventCallback callback) {
        m_event_callbacks.push_back(callback);
    }
    std::vector<EventCallback> m_event_callbacks;

public:
    std::unique_ptr<NativeWindow> m_native_window = nullptr;
    WINDOW_STATE                  m_window_state;
    WindowEventQueue              m_queue;
    InputState                    m_input_state;
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
    NativeWindow(NativeWindow&&) = default;
    auto operator=(NativeWindow&&) -> NativeWindow& = default;
    virtual ~NativeWindow() = default;

public:
    virtual auto               handle_events(bool& running) -> void = 0;
    virtual auto               set_title(const std::string& title) -> void = 0;
    [[nodiscard]] virtual auto get_title() const -> std::string = 0;
    [[nodiscard]] virtual auto get_size() const -> const v2u32& = 0;

    /// The platform window that owns this native window.
    Window* m_platform_window = nullptr;
};
} // namespace JadeFrame