#pragma once
#include "JadeFrame/prelude.h"
#include "JadeFrame/math/vec.h"

namespace JadeFrame {
class NativeWindow;

class Window {
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

    Window() = default;

    explicit Window(const Window::Desc& desc);

    auto               handle_events(bool& running) -> void;
    [[nodiscard]] auto get_window_state() const -> WINDOW_STATE;
    auto               set_title(const std::string& title) -> void;
    [[nodiscard]] auto get_title() const -> std::string;
    [[nodiscard]] auto get_size() const -> const v2u32&;

public:
    std::unique_ptr<NativeWindow> m_native_window;
    WINDOW_STATE                  m_window_state;
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
    virtual auto               handle_events(bool& running) -> void = 0;
    virtual auto               set_title(const std::string& title) -> void = 0;
    [[nodiscard]] virtual auto get_title() const -> std::string = 0;
    [[nodiscard]] virtual auto get_size() const -> const v2u32& = 0;
};
} // namespace JadeFrame