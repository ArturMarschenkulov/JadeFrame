#include "window.h"
#include "JadeFrame/platform/window_event.h"
#include "JadeFrame/utils/assert.h"
#include "JadeFrame/utils/logger.h"
#if defined(JF_PLATFORM_LINUX)
    #include "JadeFrame/platform/linux/linux_window.h"
#elif defined(JF_PLATFORM_WINDOWS)
    #include "JadeFrame/platform/windows/windows_window.h"
#else
    #error "Unsupported platform"
#endif
#include "JadeFrame/utils/logger.h"
#include <imgui/imgui.h>

namespace JadeFrame {

static auto button_jadeframe_to_imgui(const BUTTON& button) -> ImGuiMouseButton {
    switch (button) {
        case BUTTON::LEFT: return ImGuiMouseButton_Left;
        case BUTTON::RIGHT: return ImGuiMouseButton_Right;
        case BUTTON::MIDDLE: return ImGuiMouseButton_Middle;
        case BUTTON::X1:
        case BUTTON::X2: JF_ASSERT(false, "X1 and X2 are not supported by imgui");
        default:
            Logger::err("Unknown button: {}", (u32)button);
            JF_ASSERT(false, "Unknown button");
    }
}

constexpr static auto keys_jadeframe_to_imgui(const KEY& is) -> ImGuiKey_ {

    switch (is) {
        case KEY::SPACE: return ImGuiKey_Space;
        case KEY::ESCAPE: return ImGuiKey_Escape;
        case KEY::ENTER: return ImGuiKey_Enter;
        case KEY::TAB: return ImGuiKey_Tab;
        case KEY::BACKSPACE: return ImGuiKey_Backspace;
        case KEY::INSERT: return ImGuiKey_Insert;
        case KEY::DELET: return ImGuiKey_Delete;
        case KEY::RIGHT: return ImGuiKey_RightArrow;
        case KEY::LEFT: return ImGuiKey_LeftArrow;
        case KEY::DOWN: return ImGuiKey_DownArrow;
        case KEY::UP: return ImGuiKey_UpArrow;
        case KEY::PAGE_UP: return ImGuiKey_PageUp;
        case KEY::PAGE_DOWN: return ImGuiKey_PageDown;
        case KEY::HOME: return ImGuiKey_Home;
        case KEY::END: return ImGuiKey_End;
        case KEY::CAPS_LOCK: return ImGuiKey_CapsLock;
        case KEY::SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case KEY::NUM_LOCK: return ImGuiKey_NumLock;
        case KEY::PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case KEY::PAUSE: return ImGuiKey_Pause;
        case KEY::A: return ImGuiKey_A;
        case KEY::B: return ImGuiKey_B;
        case KEY::C: return ImGuiKey_C;
        case KEY::D: return ImGuiKey_D;
        case KEY::E: return ImGuiKey_E;
        case KEY::F: return ImGuiKey_F;
        case KEY::G: return ImGuiKey_G;
        case KEY::H: return ImGuiKey_H;
        case KEY::I: return ImGuiKey_I;
        case KEY::J: return ImGuiKey_J;
        case KEY::K: return ImGuiKey_K;
        case KEY::L: return ImGuiKey_L;
        case KEY::M: return ImGuiKey_M;
        case KEY::N: return ImGuiKey_N;
        case KEY::O: return ImGuiKey_O;
        case KEY::P: return ImGuiKey_P;
        case KEY::Q: return ImGuiKey_Q;
        case KEY::R: return ImGuiKey_R;
        case KEY::S: return ImGuiKey_S;
        case KEY::T: return ImGuiKey_T;
        case KEY::U: return ImGuiKey_U;
        case KEY::V: return ImGuiKey_V;
        case KEY::W: return ImGuiKey_W;
        case KEY::X: return ImGuiKey_X;
        case KEY::Y: return ImGuiKey_Y;
        case KEY::Z: return ImGuiKey_Z;
        case KEY::ZERO: return ImGuiKey_0;
        case KEY::ONE: return ImGuiKey_1;
        case KEY::TWO: return ImGuiKey_2;
        case KEY::THREE: return ImGuiKey_3;
        case KEY::FOUR: return ImGuiKey_4;
        case KEY::FIVE: return ImGuiKey_5;
        case KEY::SIX: return ImGuiKey_6;
        case KEY::SEVEN: return ImGuiKey_7;
        case KEY::EIGHT: return ImGuiKey_8;
        case KEY::NINE: return ImGuiKey_9;
        case KEY::F1: return ImGuiKey_F1;
        case KEY::F2: return ImGuiKey_F2;
        case KEY::F3: return ImGuiKey_F3;
        case KEY::F4: return ImGuiKey_F4;
        case KEY::F5: return ImGuiKey_F5;
        case KEY::F6: return ImGuiKey_F6;
        case KEY::F7: return ImGuiKey_F7;
        case KEY::F8: return ImGuiKey_F8;
        case KEY::F9: return ImGuiKey_F9;
        case KEY::F10: return ImGuiKey_F10;
        case KEY::F11: return ImGuiKey_F11;
        case KEY::F12: return ImGuiKey_F12;
        case KEY::LEFT_SHIFT: return ImGuiKey_LeftShift;
        case KEY::LEFT_CONTROL: return ImGuiKey_LeftCtrl;
        case KEY::LEFT_ALT: return ImGuiKey_LeftAlt;
        case KEY::LEFT_SUPER: return ImGuiKey_LeftSuper;
        case KEY::RIGHT_SHIFT: return ImGuiKey_RightShift;
        case KEY::RIGHT_CONTROL: return ImGuiKey_RightCtrl;
        case KEY::RIGHT_ALT: return ImGuiKey_RightAlt;
        default: {
            Logger::err("Unknown key: {}", (u32)is);
            JF_ASSERT(false, "Unknown key");
        }
    }
}

static auto imgui_handle_key(KeyEvent event) -> void {
    ImGuiIO& io = ImGui::GetIO();
    auto     imgui_key = keys_jadeframe_to_imgui(event.key);
    io.KeysDown[imgui_key] = event.type == INPUT_STATE::PRESSED;
}

static auto imgui_handle_button(ButtonEvent event) -> void {
    ImGuiIO& io = ImGui::GetIO();
    auto     imgui_button = button_jadeframe_to_imgui(event.button);
    io.MouseDown[imgui_button] = event.type == INPUT_STATE::PRESSED;
}

static auto imgui_event_callback(const WindowEvent& event) -> void {
    ImGuiIO& io = ImGui::GetIO();
    switch (event.type) {
        case WindowEvent::TYPE::KEY: {
            KeyEvent key_event = event.key_event;
            auto     imgui_key = keys_jadeframe_to_imgui(key_event.key);
            io.KeysDown[imgui_key] = key_event.type == INPUT_STATE::PRESSED;
        } break;
        case WindowEvent::TYPE::BUTTON: {
            ButtonEvent button_event = event.button_event;
            auto        imgui_button = button_jadeframe_to_imgui(button_event.button);
            io.MouseDown[imgui_button] = button_event.type == INPUT_STATE::PRESSED;
        } break;
        default: break;
    }
}

Window::Window(Window&& other) noexcept {
    m_native_window = std::exchange(other.m_native_window, nullptr);
    m_native_window->m_platform_window = this;
}

auto Window::operator=(Window&& other) noexcept -> Window& {
    m_native_window = std::exchange(other.m_native_window, nullptr);
    m_native_window->m_platform_window = this;

    return *this;
}

static auto debug_event_callback(const WindowEvent& e) -> void {
    switch (e.type) {
        case WindowEvent::TYPE::KEY: {
            KeyEvent key_event = e.key_event;
            Logger::debug(
                "Key {} was {}",
                (u32)key_event.key,
                key_event.type == INPUT_STATE::PRESSED ? "pressed" : "released"
            );
        } break;
        case WindowEvent::TYPE::BUTTON: {
            ButtonEvent button_event = e.button_event;
            Logger::debug(
                "Button {} was {}",
                (u32)button_event.button,
                button_event.type == INPUT_STATE::PRESSED ? "pressed" : "released"
            );
        } break;
        case WindowEvent::TYPE::MOUSE: {
            MouseEvent mouse_event = e.mouse_event;
            Logger::debug(
                "Mouse moved to: x: {}, y: {}", mouse_event.m_x, mouse_event.m_y
            );
        } break;
    }
}

Window::Window(const Window::Desc& desc) {
#if defined(JF_PLATFORM_LINUX)
    m_native_window = std::make_unique<X11_NativeWindow>(X11_NativeWindow::create(desc));
    m_native_window->m_platform_window = this;
#elif defined(JF_PLATFORM_WINDOWS)
    m_native_window = std::make_unique<win32::NativeWindow>(desc);
    m_native_window->m_platform_window = this;
#else
    #error "Unsupported platform"
#endif

    this->add_event_callback(debug_event_callback);
    // TODO(artur): For now here, but later on would be added by the caller if desired.
    this->add_event_callback(imgui_event_callback);
}

auto Window::handle_events(bool& running) -> void {
    m_native_window->handle_events(running);
    m_input_state.update();

    while (!m_queue.is_empty()) {
        WindowEvent event = m_queue.pop();
        switch (event.type) {
            case WindowEvent::TYPE::KEY: {
                KeyEvent key_event = event.key_event;
                u32      key_index = static_cast<u32>(key_event.key);
                m_input_state.m_curr_key_state[key_index] = key_event.type;
            } break;
            case WindowEvent::TYPE::BUTTON: {
                ButtonEvent button_event = event.button_event;
                u32         button_index = static_cast<u32>(button_event.button);
                m_input_state.m_curr_button_state[button_index] = button_event.type;
            } break;
            case WindowEvent::TYPE::MOUSE: {
                MouseEvent mouse_event = event.mouse_event;
                m_input_state.m_mouse_pos = v2::create(mouse_event.m_x, mouse_event.m_y);
            } break;
        }
        for (const auto& callback : m_event_callbacks) { callback(event); }
    }
}

auto Window::get_window_state() const -> WINDOW_STATE { return m_window_state; }

auto Window::set_title(const std::string& title) -> void {
    m_native_window->set_title(title);
}

auto Window::get_title() const -> std::string { return m_native_window->get_title(); }

auto Window::get_size() const -> const v2u32& { return m_native_window->get_size(); }

auto InputState::is_key_down(const KEY key) const -> bool {
    u32         key_0 = static_cast<u32>(key);
    INPUT_STATE curr = m_curr_key_state[key_0];

    bool is_pressed = (curr == INPUT_STATE::PRESSED);
    return is_pressed;
}

auto InputState::is_key_up(const KEY key) const -> bool {
    u32         key_0 = static_cast<u32>(key);
    INPUT_STATE curr = m_curr_key_state[key_0];

    bool is_current_released = (curr == INPUT_STATE::RELEASED);
    return is_current_released;
}

auto InputState::is_key_pressed(const KEY key) const -> bool {
    u32         key_0 = static_cast<u32>(key);
    INPUT_STATE curr = m_curr_key_state[key_0];
    INPUT_STATE prev = m_prev_key_state[key_0];

    bool is_changed = (curr != prev);
    bool is_pressed = (curr == INPUT_STATE::PRESSED);
    return is_changed && is_pressed;
}

auto InputState::is_key_released(const KEY key) const -> bool {
    u32         key_0 = static_cast<u32>(key);
    INPUT_STATE curr = m_curr_key_state[key_0];
    INPUT_STATE prev = m_prev_key_state[key_0];

    bool is_changed = (curr != prev);
    bool is_released = (curr == INPUT_STATE::RELEASED);
    return is_changed && is_released;
}

auto InputState::is_button_down(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_button_state[button_0];

    bool is_pressed = (curr == INPUT_STATE::PRESSED);
    return is_pressed;
}

auto InputState::is_button_up(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_button_state[button_0];

    bool is_released = (curr == INPUT_STATE::RELEASED);
    return is_released;
}

auto InputState::is_button_pressed(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_button_state[button_0];
    INPUT_STATE prev = m_prev_button_state[button_0];

    bool is_changed = (curr != prev);
    bool is_pressed = (curr == INPUT_STATE::PRESSED);
    return is_changed && is_pressed;
}

auto InputState::is_button_released(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_button_state[button_0];
    INPUT_STATE prev = m_prev_button_state[button_0];

    bool is_changed = (curr != prev);
    bool is_released = (curr == INPUT_STATE::RELEASED);
    return is_changed && is_released;
}

auto InputState::get_mouse_position() const -> v2 { return m_mouse_pos; }

auto InputState::update() -> void {
    m_prev_key_state = m_curr_key_state;
    m_prev_button_state = m_curr_button_state;
}

} // namespace JadeFrame