#include "window.h"
#include "JadeFrame/platform/window_event.h"
#if defined(JF_PLATFORM_LINUX)
    #include "JadeFrame/platform/linux/linux_window.h"
#elif defined(JF_PLATFORM_WINDOWS)
    #include "JadeFrame/platform/windows/windows_window.h"
#else
    #error "Unsupported platform"
#endif
#include <algorithm>

namespace JadeFrame {

Window::Window(Window&& other) noexcept {
    m_native_window = std::exchange(other.m_native_window, nullptr);
    m_window_state = other.m_window_state;
    m_queue = std::move(other.m_queue);
    m_input_state = other.m_input_state;
    m_event_callbacks = std::move(other.m_event_callbacks);
    m_next_event_callback_id = other.m_next_event_callback_id;
    if (m_native_window != nullptr) { m_native_window->m_platform_window = this; }
}

auto Window::operator=(Window&& other) noexcept -> Window& {
    m_native_window = std::exchange(other.m_native_window, nullptr);
    m_window_state = other.m_window_state;
    m_queue = std::move(other.m_queue);
    m_input_state = other.m_input_state;
    m_event_callbacks = std::move(other.m_event_callbacks);
    m_next_event_callback_id = other.m_next_event_callback_id;
    if (m_native_window != nullptr) { m_native_window->m_platform_window = this; }

    return *this;
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
    m_window_state = desc.window_state;
}

auto Window::add_event_callback(EventCallback callback) -> EventCallbackID {
    const EventCallbackID callback_id = m_next_event_callback_id++;
    m_event_callbacks.emplace_back(callback_id, std::move(callback));
    return callback_id;
}

auto Window::remove_event_callback(EventCallbackID callback_id) -> void {
    auto it = std::remove_if(
        m_event_callbacks.begin(),
        m_event_callbacks.end(),
        [callback_id](const auto& entry) { return entry.first == callback_id; }
    );
    m_event_callbacks.erase(it, m_event_callbacks.end());
}

auto Window::handle_events(bool& is_running) -> void {
    m_native_window->handle_events(is_running);
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
            case WindowEvent::TYPE::RESIZE: {
                const WindowResizeEvent resize_event = event.resize_event;
                (void)resize_event;
            } break;
            case WindowEvent::TYPE::CLOSE: {
                is_running = false;
            } break;
        }
        for (const auto& [id, callback] : m_event_callbacks) {
            (void)id;
            callback(event);
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
