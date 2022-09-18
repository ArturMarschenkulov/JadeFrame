#include "pch.h"
#include "windows_input_manager.h"

#include "JadeFrame/base_app.h" // For `Instance::get_singleton()`
//#include "JadeFrame/gui.h"
#include <Windows.h>
#include <windowsx.h>
#include "windows_message_map.h" // for WindowMessage struct

namespace JadeFrame {
namespace win32 {
/*
        KEY INPUT
*/
std::array<INPUT_STATE, 512> InputManager::m_current_key_state = {};
std::array<INPUT_STATE, 512> InputManager::m_previous_key_state = {};

auto InputManager::handle_input() -> void {
    for (size_t i = 0; i < m_current_key_state.size(); i++) { m_previous_key_state[i] = m_current_key_state[i]; }
}

auto InputManager::key_callback(const EventMessage& wm) -> void {
    auto hwnd = wm.hWnd;
    // window_message.message;
    auto wParam = wm.wParam;
    auto lParam = wm.lParam;

    u64 key_code = wParam;
    // int64_t bit_29 = (lParam >> 29) & 1; // 1 == system key
    // int64_t bit_30 = (lParam >> 30) & 1; // 1 == repeatedly pressed
    i64 bit_31 = (lParam >> 31) & 1; // 0 == pressed, 1 == released

    // bool b_is_system_key = (bit_29 == 1);
    // bool b_is_repeated = (bit_30 == 1);
    bool b_is_pressed = (bit_31 == 0);

    m_current_key_state[key_code] = static_cast<INPUT_STATE>(b_is_pressed);
    // ImGuiIO& io = ImGui::GetIO();
    // io.KeysDown[key_code] = b_is_pressed;


    // TODO: Try to extract that to somewhere else. So th
    if (m_current_key_state[static_cast<i32>(KEY::ESCAPE)] == INPUT_STATE::PRESSED) {
        if (::MessageBoxW(hwnd, L"Quit through ESC?", L"My application", MB_OKCANCEL) == IDOK) {
            Logger::log("WinInputManager::key_callback(); WM_QUIT");
            //__debugbreak();
            Instance::get_singleton()->m_current_app_p->m_is_running = false;
            ::PostQuitMessage(0);
            // DestroyWindow(hwnd);
        }
        // JadeFrame::get_singleton()->m_current_app->m_is_running = false;
        //::PostQuitMessage(0);
    }
}
// auto InputManager::key_callback2(i64 lParam, u64 wParam, u32 message) -> void {
//	u64 key_code = wParam;
//
//	//int64_t bit_29 = (lParam >> 29) & 1; // 1 == system key (basically ALT key + some key)
//	//int64_t bit_30 = (lParam >> 30) & 1; // 1 == repeatedly pressed
//	i64 bit_31 = (lParam >> 31) & 1; // 0 == pressed, 1 == released
//
//	//bool b_is_system_key = (bit_29 == 1);
//	//bool b_is_repeated = (bit_30 == 1);
//	bool b_is_pressed = (bit_31 == 0);
//
//	KeyEvent::TYPE key_event_type = b_is_pressed ? KeyEvent::TYPE::PRESSED : KeyEvent::TYPE::RELEASED;
//	KeyEvent key_event = { key_event_type, key_code };
//
//	std::vector<KeyEvent> key_events;
//	key_events.push_back(key_event);
//
//	if (key_event.type == KeyEvent::TYPE::PRESSED) {
//		m_current_key_state[key_code] = INPUT_STATE::PRESSED;
//	} else if (key_event.type == KeyEvent::TYPE::RELEASED) {
//		m_current_key_state[key_code] = INPUT_STATE::RELEASED;
//	} else {
//		__debugbreak();
//	}
//
//
//	//TODO: Try to extract that to somewhere else.
//	if (m_current_key_state[static_cast<i32>(KEY::ESCAPE)] == INPUT_STATE::PRESSED) {
//		JadeFrame::get_singleton()->m_current_app_p->m_is_running = false;
//		::PostQuitMessage(0);
//	}
// }
auto InputManager::char_callback(const EventMessage& wm) -> void {
    // window_message.hWnd;
    // window_message.message;
    auto wParam = wm.wParam;
    // auto lParam = window_message.lParam;

    // int64_t bit_29 = (lParam >> 29) & 1; // 1 == system key
    // int64_t bit_30 = (lParam >> 30) & 1; // 1 == repeatedly pressed
    // int64_t bit_31 = (lParam >> 31) & 1; // 0 == pressed, 1 == released
    // ImGuiIO& io = ImGui::GetIO();
    if (wParam > 0 && wParam < 0x10000) {
        // io.AddInputCharacter((u16)wParam);
    }
}

auto InputManager::is_key_down(const KEY key) -> bool {
    i32  key_0 = static_cast<i32>(key);
    bool is_current_pressed = (m_current_key_state[key_0] == INPUT_STATE::PRESSED);
    return is_current_pressed ? true : false;
}
auto InputManager::is_key_up(const KEY key) -> bool {
    i32  key_0 = static_cast<i32>(key);
    bool is_current_released = (m_current_key_state[key_0] == INPUT_STATE::RELEASED);
    return is_current_released ? true : false;
}
auto InputManager::is_key_pressed(const KEY key) -> bool {
    i32  key_0 = static_cast<i32>(key);
    bool is_current_changed = (m_current_key_state[key_0] != m_previous_key_state[key_0]);
    bool is_current_pressed = (m_current_key_state[key_0] == INPUT_STATE::PRESSED);
    return (is_current_changed && is_current_pressed) ? true : false;
}
auto InputManager::is_key_released(const KEY key) -> bool {
    i32  key_0 = static_cast<i32>(key);
    bool is_current_changed = (m_current_key_state[key_0] != m_previous_key_state[key_0]);
    bool is_current_released = (m_current_key_state[key_0] == INPUT_STATE::RELEASED);
    return (is_current_changed && is_current_released) ? true : false;
}


/*
        MOUSE INPUT
*/
std::array<INPUT_STATE, 3> InputManager::m_current_mouse_button_state = {};
std::array<INPUT_STATE, 3> InputManager::m_previous_mouse_button_state = {};

v2 InputManager::m_mouse_posiition{};

static auto convert_buttons_from_JF_to_imgui(BUTTON button) -> i32 {
    i32 result;
    switch (button) {
        case BUTTON::LEFT: result = 0; break;
        case BUTTON::RIGHT: result = 1; break;
        case BUTTON::MIDDLE: result = 4; break;
        default:
            result = -1;
            assert(false);
            break;
    }
    return result;
}
// static auto convert_keys_from_JF_to_imgui(KEY button) -> i32 {
//	i32 result;
//	switch (button) {
//		//case BUTTON::LEFT: result = 0; break;
//		//case BUTTON::RIGHT: result = 1; break;
//		//case BUTTON::MIDDLE: result = 4; break;
//		default: result = -1; __debugbreak(); break;
//	}
//	return result;
// }

// auto InputManager::mouse_button_callback(i64 lParam, i64 wParam, i32 message) -> void {
auto InputManager::mouse_button_callback(const EventMessage& wm) -> void {
    // window_message.hWnd;
    auto message = wm.message;
    // auto wParam = window_message.wParam;
    auto lParam = wm.lParam;

    // ImGuiIO& io = ImGui::GetIO();

    // TODO: The X2 is used because it is sufficiently rare. Though maybe thing of another way to remove the warning of
    // no initialization.
    BUTTON button = BUTTON::X2;
    switch (message) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK: {
            if (message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) { button = BUTTON::LEFT; }
            if (message == WM_RBUTTONDOWN || message == WM_RBUTTONDBLCLK) { button = BUTTON::RIGHT; }
            if (message == WM_MBUTTONDOWN || message == WM_MBUTTONDBLCLK) { button = BUTTON::MIDDLE; }
            m_current_mouse_button_state[static_cast<i32>(button)] = INPUT_STATE::PRESSED;
            // io.MouseDown[convert_buttons_from_JF_to_imgui(button)] = true;


        } break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            if (message == WM_LBUTTONUP) { button = BUTTON::LEFT; }
            if (message == WM_RBUTTONUP) { button = BUTTON::RIGHT; }
            if (message == WM_MBUTTONUP) { button = BUTTON::MIDDLE; }
            m_current_mouse_button_state[static_cast<i32>(button)] = INPUT_STATE::RELEASED;
            // io.MouseDown[convert_buttons_from_JF_to_imgui(button)] = false;
        } break;
        case WM_MOUSEMOVE: {
            i32 mposx = GET_X_LPARAM(lParam);
            i32 mposy = GET_Y_LPARAM(lParam);
            m_mouse_posiition.x = static_cast<f32>(mposx);
            m_mouse_posiition.y = static_cast<f32>(mposy);
        } break;
    }
    i32 mposx = GET_X_LPARAM(lParam);
    i32 mposy = GET_Y_LPARAM(lParam);
    m_mouse_posiition.x = static_cast<f32>(mposx);
    m_mouse_posiition.y = static_cast<f32>(mposy);
    if (is_key_down(KEY::L)) { __debugbreak(); }
    // io.DeltaTime = 1.0f / 60.0f;
}

auto InputManager::is_button_down(const BUTTON button) const -> bool {
    i32  button_0 = static_cast<i32>(button);
    bool is_current_pressed = (m_current_key_state[button_0] == INPUT_STATE::PRESSED);
    return is_current_pressed ? true : false;
}

auto InputManager::is_button_up(const BUTTON button) const -> bool {
    i32  button_0 = static_cast<i32>(button);
    bool is_current_released = (m_current_key_state[button_0] == INPUT_STATE::RELEASED);
    return is_current_released ? true : false;
}

auto InputManager::is_button_pressed(const BUTTON button) const -> bool {
    i32  button_0 = static_cast<i32>(button);
    bool is_current_changed = (m_current_key_state[button_0] != m_previous_key_state[button_0]);
    bool is_current_pressed = (m_current_key_state[button_0] == INPUT_STATE::PRESSED);
    return (is_current_changed && is_current_pressed) ? true : false;
}

auto InputManager::is_button_released(const BUTTON button) const -> bool {
    i32  button_0 = static_cast<i32>(button);
    bool is_current_changed = (m_current_key_state[button_0] != m_previous_key_state[button_0]);
    bool is_current_released = (m_current_key_state[button_0] == INPUT_STATE::RELEASED);
    return (is_current_changed && is_current_released) ? true : false;
}

auto InputManager::get_mouse_position() const -> v2 { return m_mouse_posiition; }
} // namespace win32
} // namespace JadeFrame