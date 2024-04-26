#include "pch.h"
#include "windows_input_manager.h"

#include "JadeFrame/base_app.h" // For `Instance::get_singleton()`
// #include "JadeFrame/gui.h"
#include <Windows.h>
#include <windowsx.h>
#include "windows_message_map.h" // for WindowMessage struct

namespace JadeFrame {
namespace win32 {
/*
        KEY INPUT
*/
std::array<INPUT_STATE, static_cast<u32>(KEY::MAX)> InputManager::m_curr_key_state = {};
std::array<INPUT_STATE, static_cast<u32>(KEY::MAX)> InputManager::m_prev_key_state = {};

auto InputManager::translate_button_code(const u64 button_code) -> BUTTON {
    BUTTON result;
    switch (button_code) {
        case VK_LBUTTON: result = BUTTON::LEFT; break;
        case VK_RBUTTON: result = BUTTON::RIGHT; break;
        case VK_MBUTTON: result = BUTTON::MIDDLE; break;
        case VK_XBUTTON1: result = BUTTON::X1; break;
        case VK_XBUTTON2: result = BUTTON::X2; break;
    }
    return result;
}

auto InputManager::translate_key_code(const u64 key_code) -> KEY {
    KEY result;
    switch (key_code) {
        case VK_SPACE: result = KEY::SPACE; break;
        case VK_ESCAPE: result = KEY::ESCAPE; break;
        case VK_RETURN: result = KEY::ENTER; break;
        case VK_TAB: result = KEY::TAB; break;
        case VK_BACK: result = KEY::BACKSPACE; break;
        case VK_INSERT: result = KEY::INSERT; break;
        case VK_DELETE: result = KEY::DELET; break;
        case VK_RIGHT: result = KEY::RIGHT; break;
        case VK_LEFT: result = KEY::LEFT; break;
        case VK_DOWN: result = KEY::DOWN; break;
        case VK_UP: result = KEY::UP; break;
        case VK_PRIOR: result = KEY::PAGE_UP; break;
        case VK_NEXT: result = KEY::PAGE_DOWN; break;
        case VK_HOME: result = KEY::HOME; break;
        case VK_END: result = KEY::END; break;
        case VK_CAPITAL: result = KEY::CAPS_LOCK; break;
        case VK_SCROLL: result = KEY::SCROLL_LOCK; break;
        case VK_NUMLOCK: result = KEY::NUM_LOCK; break;
        case VK_SNAPSHOT: result = KEY::PRINT_SCREEN; break;
        case VK_PAUSE: result = KEY::PAUSE; break;
        case VK_F1: result = KEY::F1; break;
        case VK_F2: result = KEY::F2; break;
        case VK_F3: result = KEY::F3; break;
        case VK_F4: result = KEY::F4; break;
        case VK_F5: result = KEY::F5; break;
        case VK_F6: result = KEY::F6; break;
        case VK_F7: result = KEY::F7; break;
        case VK_F8: result = KEY::F8; break;
        case VK_F9: result = KEY::F9; break;
        case VK_F10: result = KEY::F10; break;
        case VK_F11: result = KEY::F11; break;
        case VK_F12: result = KEY::F12; break;
        case VK_LSHIFT: result = KEY::LEFT_SHIFT; break;
        case VK_LCONTROL: result = KEY::LEFT_CONTROL; break;
        case VK_LMENU: result = KEY::LEFT_ALT; break;
        case VK_RSHIFT: result = KEY::RIGHT_SHIFT; break;
        case VK_RCONTROL: result = KEY::RIGHT_CONTROL; break;
        case VK_RMENU: result = KEY::RIGHT_ALT; break;
        case VK_LWIN: result = KEY::LEFT_SUPER; break;
        case VK_CONTROL: result = KEY::LEFT_CONTROL; break;

        case VK_OEM_1: result = KEY::OEM_1; break;
        case VK_OEM_PLUS: result = KEY::OEM_PLUS; break;
        case VK_OEM_COMMA: result = KEY::OEM_COMMA; break;
        case VK_OEM_MINUS: result = KEY::OEM_MINUS; break;
        case VK_OEM_PERIOD: result = KEY::OEM_PERIOD; break;
        case VK_OEM_2: result = KEY::OEM_2; break;
        case VK_OEM_3: result = KEY::OEM_3; break;
        case VK_OEM_4: result = KEY::OEM_4; break;
        case VK_OEM_5: result = KEY::OEM_5; break;
        case VK_OEM_6: result = KEY::OEM_6; break;
        case VK_OEM_7: result = KEY::OEM_7; break;
        case VK_OEM_8: result = KEY::OEM_8; break;
        case VK_OEM_AX: result = KEY::OEM_AX; break;
        case VK_OEM_102: result = KEY::OEM_102; break;
        case VK_ICO_HELP: result = KEY::ICO_HELP; break;
        case VK_ICO_00: result = KEY::ICO_00; break;

        case 48: result = KEY::ZERO; break;
        case 49: result = KEY::ONE; break;
        case 50: result = KEY::TWO; break;
        case 51: result = KEY::THREE; break;
        case 52: result = KEY::FOUR; break;
        case 53: result = KEY::FIVE; break;
        case 54: result = KEY::SIX; break;
        case 55: result = KEY::SEVEN; break;
        case 56: result = KEY::EIGHT; break;
        case 57: result = KEY::NINE; break;

        case 65: result = KEY::A; break;
        case 66: result = KEY::B; break;
        case 67: result = KEY::C; break;
        case 68: result = KEY::D; break;
        case 69: result = KEY::E; break;
        case 70: result = KEY::F; break;
        case 71: result = KEY::G; break;
        case 72: result = KEY::H; break;
        case 73: result = KEY::I; break;
        case 74: result = KEY::J; break;
        case 75: result = KEY::K; break;
        case 76: result = KEY::L; break;
        case 77: result = KEY::M; break;
        case 78: result = KEY::N; break;
        case 79: result = KEY::O; break;
        case 80: result = KEY::P; break;
        case 81: result = KEY::Q; break;
        case 82: result = KEY::R; break;
        case 83: result = KEY::S; break;
        case 84: result = KEY::T; break;
        case 85: result = KEY::U; break;
        case 86: result = KEY::V; break;
        case 87: result = KEY::W; break;
        case 88: result = KEY::X; break;
        case 89: result = KEY::Y; break;
        case 90: result = KEY::Z; break;
        default: Logger::err("Unknown key code: {}", key_code); assert(false);
    }
    return result;
}

auto InputManager::handle_input() -> void {
    for (size_t i = 0; i < m_curr_key_state.size(); i++) {
        m_prev_key_state[i] = m_curr_key_state[i];
    }
}

auto InputManager::key_callback(const EventMessage& wm) -> void {

    /*
        The lParam parameter of a keystroke message contains additional information about
       the keystroke that generated the message. This information includes the repeat
       count, the scan code, the extended-key flag, the context code, the previous
       key-state flag, and the transition-state flag. The following illustration shows the
       locations of these flags and values in the lParam parameter.

        31: Transition state flag
        30: Previous key state flag
        29: Context code
        28-25: Reserved
        24: Extended key flag
        23-16: OEM scan code
        15-0: Repeat count
    */
    HWND   hwnd = wm.hWnd;
    UINT   msg = wm.message;
    WPARAM wParam = wm.wParam;
    LPARAM lParam = wm.lParam;

    bool is_sys_stroke = msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP;
    bool is_down = WM_KEYDOWN == msg || WM_SYSKEYDOWN == msg;

    WORD virtual_key = LOWORD(wParam);

    WORD key_flags = HIWORD(lParam);
    BYTE scan_code = LOBYTE(key_flags);

    WORD repeat_count = LOWORD(lParam); // NOTE: For what can it be used?

#if 0
    i64 bit_24 = (wm.lParam >> 24) & 1; // extended key flag
    i64 bit_29 = (wm.lParam >> 29) & 1; // 1 == system key
    i64 bit_30 = (wm.lParam >> 30) & 1; // 1 == repeatedly pressed
    i64 bit_31 = (wm.lParam >> 31) & 1; // 0 == pressed, 1 == released

    // bool b_is_system_key = (bit_29 == 1);
    // bool b_is_repeated = (bit_30 == 1);
    // bool b_is_pressed = (bit_31 == 0);

    bool is_released = (bit_31 == 1);
    bool was_down = (bit_30 == 1);
    bool is_extended_key = (bit_24 == 1);
    if (is_extended_key) { scan_code = MAKEWORD(scan_code, 0xE0); }
#else
    bool is_extended_key = (key_flags & KF_EXTENDED) == KF_EXTENDED;
    if (is_extended_key) { scan_code = MAKEWORD(scan_code, 0xE0); }
    bool is_released = HIWORD(lParam) & KF_UP;
    bool was_down = HIWORD(lParam) & KF_REPEAT;
#endif

    // sanity checks
    {
        if (msg == WM_KEYUP || msg == WM_SYSKEYUP) { assert(repeat_count == 1); }
        if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) { assert(is_released == false); }
        if (msg == WM_KEYUP || msg == WM_SYSKEYUP) { assert(is_released = true); }

        auto mapped_vk_code = MapVirtualKey(scan_code, MAPVK_VSC_TO_VK_EX);
        if (mapped_vk_code != virtual_key) {
            Logger::warn(
                "vk code and mapped scan code don't match\n{}\n{}",
                virtual_key,
                mapped_vk_code
            );
        }
    }

    // if (is_system_stroke == true) {
    //     ::DefWindowProc(hwnd, msg, wParam, lParam);
    // }

    switch (virtual_key) {
        case VK_SHIFT:   // converts to VK_LSHIFT or VK_RSHIFT
        case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
        case VK_MENU:    // converts to VK_LMENU or VK_RMENU
            virtual_key = LOWORD(MapVirtualKey(virtual_key, MAPVK_VSC_TO_VK_EX));
            break;
    }
    KEY jf_keycode = translate_key_code(virtual_key);

    m_curr_key_state[static_cast<u32>(jf_keycode)] =
        static_cast<INPUT_STATE>(!is_released);
    // ImGuiIO& io = ImGui::GetIO();
    // io.KeysDown[key_code] = b_is_pressed;

    // TODO: Try to extract that to somewhere else. So th
    if (m_curr_key_state[static_cast<u32>(KEY::ESCAPE)] == INPUT_STATE::PRESSED) {
        if (::MessageBoxW(
                wm.hWnd, L"Quit through ESC?", L"My application", MB_OKCANCEL
            ) == IDOK) {
            Instance::get_singleton()->m_current_app_p->m_is_running = false;
            ::PostQuitMessage(0);
            // DestroyWindow(hwnd);
        }
        // JadeFrame::get_singleton()->m_current_app->m_is_running = false;
        //::PostQuitMessage(0);
    }
}

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
    i32         key_0 = static_cast<u32>(key);
    INPUT_STATE curr = m_curr_key_state[key_0];

    bool is_pressed = (curr == INPUT_STATE::PRESSED);
    return is_pressed ? true : false;
}

auto InputManager::is_key_up(const KEY key) -> bool {
    i32         key_0 = static_cast<u32>(key);
    INPUT_STATE curr = m_curr_key_state[key_0];

    bool is_current_released = (curr == INPUT_STATE::RELEASED);
    return is_current_released ? true : false;
}

auto InputManager::is_key_pressed(const KEY key) -> bool {
    i32         key_0 = static_cast<u32>(key);
    INPUT_STATE curr = m_curr_key_state[key_0];
    INPUT_STATE prev = m_prev_key_state[key_0];

    bool is_changed = (m_curr_key_state[key_0] != m_prev_key_state[key_0]);
    bool is_pressed = (m_curr_key_state[key_0] == INPUT_STATE::PRESSED);
    return (is_changed && is_pressed) ? true : false;
}

auto InputManager::is_key_released(const KEY key) -> bool {
    u32         key_0 = static_cast<u32>(key);
    INPUT_STATE curr = m_curr_key_state[key_0];
    INPUT_STATE prev = m_prev_key_state[key_0];

    bool is_changed = (curr != prev);
    bool is_released = (curr == INPUT_STATE::RELEASED);
    return (is_changed && is_released) ? true : false;
}

/*
        MOUSE INPUT
*/
std::array<INPUT_STATE, static_cast<u32>(BUTTON::MAX)>
    InputManager::m_current_mouse_button_state = {};
std::array<INPUT_STATE, static_cast<u32>(BUTTON::MAX)>
    InputManager::m_previous_mouse_button_state = {};

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
//		default: result = -1; assert(false); break;
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

    BUTTON button = BUTTON::MAX;
    switch (message) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK: {
            if (message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) {
                button = BUTTON::LEFT;
            }
            if (message == WM_RBUTTONDOWN || message == WM_RBUTTONDBLCLK) {
                button = BUTTON::RIGHT;
            }
            if (message == WM_MBUTTONDOWN || message == WM_MBUTTONDBLCLK) {
                button = BUTTON::MIDDLE;
            }

            m_current_mouse_button_state[static_cast<u32>(button)] = INPUT_STATE::PRESSED;
            // io.MouseDown[convert_buttons_from_JF_to_imgui(button)] = true;

        } break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            if (message == WM_LBUTTONUP) { button = BUTTON::LEFT; }
            if (message == WM_RBUTTONUP) { button = BUTTON::RIGHT; }
            if (message == WM_MBUTTONUP) { button = BUTTON::MIDDLE; }
            m_current_mouse_button_state[static_cast<i32>(button)] =
                INPUT_STATE::RELEASED;
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
    if (is_key_down(KEY::L)) { assert(false); }
    // io.DeltaTime = 1.0f / 60.0f;
}

auto InputManager::is_button_down(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_key_state[button_0];

    bool is_pressed = (curr == INPUT_STATE::PRESSED);
    return is_pressed ? true : false;
}

auto InputManager::is_button_up(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_key_state[button_0];

    bool is_released = (curr == INPUT_STATE::RELEASED);
    return is_released ? true : false;
}

auto InputManager::is_button_pressed(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_key_state[button_0];
    INPUT_STATE prev = m_prev_key_state[button_0];

    bool is_changed = (curr != prev);
    bool is_pressed = (curr == INPUT_STATE::PRESSED);
    return (is_changed && is_pressed) ? true : false;
}

auto InputManager::is_button_released(const BUTTON button) const -> bool {
    i32 button_0 = static_cast<i32>(button);

    INPUT_STATE curr = m_curr_key_state[button_0];
    INPUT_STATE prev = m_prev_key_state[button_0];

    bool is_changed = (curr != prev);
    bool is_released = (curr == INPUT_STATE::RELEASED);
    return (is_changed && is_released) ? true : false;
}

auto InputManager::get_mouse_position() const -> v2 { return m_mouse_posiition; }
} // namespace win32
} // namespace JadeFrame