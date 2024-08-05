#include "window.h"
#include "JadeFrame/utils/logger.h"
#if defined(JF_PLATFORM_LINUX)
    #include "JadeFrame/platform/linux/linux_window.h"
#elif defined(JF_PLATFORM_WINDOWS)
    #include "JadeFrame/platform/windows/windows_window.h"
#else
    #error "Unsupported platform"
#endif

namespace JadeFrame {

Window::Window(const Window::Desc& desc) {
#if defined(JF_PLATFORM_LINUX)
    m_native_window = std::make_unique<X11_NativeWindow>(X11_NativeWindow::create(desc));
    m_native_window->m_platform_window = this;
#elif defined(JF_PLATFORM_WINDOWS)
    m_native_window = std::make_unique<win32::NativeWindow>(desc);
#else
    #error "Unsupported platform"
#endif
}

auto Window::handle_events(bool& running) -> void {
    m_native_window->handle_events(running);
    m_input_state.update();

    for (u32 i = 0; i < m_queue.m_queue.size(); i++) {
        auto event = m_queue.pop();
        switch (event.type) {
            case WindowEvent::TYPE::KEY: {
                auto key_event = event.key_event;
                auto key_index = static_cast<u32>(key_event.key);
                m_input_state.m_curr_key_state[key_index] = key_event.type;
                Logger::info(
                    "Key {} was {}",
                    (u32)key_event.key,
                    key_event.type == INPUT_STATE::PRESSED ? "pressed" : "released"
                );
            }
            case WindowEvent::TYPE::BUTTON: {
                auto button_event = event.button_event;
                auto button_index = static_cast<u32>(button_event.button);
                m_input_state.m_curr_key_state[button_index] = button_event.type;
                Logger::info(
                    "Button {} was {}",
                    (u32)button_event.button,
                    button_event.type == INPUT_STATE::PRESSED ? "pressed" : "released"
                );
            } break;
            case WindowEvent::TYPE::MOUSE: {
                auto mouse_event = event.mouse_event;
                m_input_state.m_mouse_pos = v2::create(mouse_event.m_x, mouse_event.m_y);
                Logger::info(
                    "Mouse moved to: x: {}, y: {}", mouse_event.m_x, mouse_event.m_y
                );
            } break;
        }
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

    bool is_changed = (m_curr_key_state[key_0] != m_prev_key_state[key_0]);
    bool is_pressed = (m_curr_key_state[key_0] == INPUT_STATE::PRESSED);
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

    INPUT_STATE curr = m_curr_key_state[button_0];

    bool is_pressed = (curr == INPUT_STATE::PRESSED);
    return is_pressed;
}

auto InputState::is_button_up(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_key_state[button_0];

    bool is_released = (curr == INPUT_STATE::RELEASED);
    return is_released;
}

auto InputState::is_button_pressed(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_key_state[button_0];
    INPUT_STATE prev = m_prev_key_state[button_0];

    bool is_changed = (curr != prev);
    bool is_pressed = (curr == INPUT_STATE::PRESSED);
    return is_changed && is_pressed;
}

auto InputState::is_button_released(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_key_state[button_0];
    INPUT_STATE prev = m_prev_key_state[button_0];

    bool is_changed = (curr != prev);
    bool is_released = (curr == INPUT_STATE::RELEASED);
    return is_changed && is_released;
}

auto InputState::get_mouse_position() const -> v2 {
    return {};
}

auto InputState::update() -> void {
    m_prev_key_state = m_curr_key_state;
    m_prev_button_state = m_curr_button_state;
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
} // namespace JadeFrame