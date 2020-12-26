#include "Input.h"
#include "BaseApp.h"
#include <iostream>
#include "GUI.h"
#include "Windows.h"
#include <windowsx.h>



/*
	KEY INPUT
*/
std::array<INPUT_STATE, 512> WinInputManager::m_current_key_state = {};
std::array<INPUT_STATE, 512> WinInputManager::m_previous_key_state = {};

auto WinInputManager::handle_input() -> void {
	for (size_t i = 0; i < m_current_key_state.size(); i++) {
		m_previous_key_state[i] = m_current_key_state[i];
	}
}

auto WinInputManager::key_callback(int64_t lParam, uint64_t wParam, uint32_t message) -> void {
	uint64_t key_code = wParam;
	int64_t bit_29 = (lParam >> 29) & 1; // 1 == system key
	int64_t bit_30 = (lParam >> 30) & 1; // 1 == repeatedly pressed
	int64_t bit_31 = (lParam >> 31) & 1; // 0 == pressed, 1 == released

	bool b_is_system_key = (bit_29 == 1);
	bool b_is_repeated = (bit_30 == 1);
	bool b_is_pressed = (bit_31 == 0);

	m_current_key_state[key_code] = static_cast<INPUT_STATE>(b_is_pressed);


	//TODO: Try to extract that to somewhere else.
	if (m_current_key_state[static_cast<int>(KEY::ESCAPE)] == INPUT_STATE::PRESSED) {
		BaseApp::get_instance()->m_is_running = false;
		::PostQuitMessage(0);
	}
}
auto WinInputManager::key_callback2(int64_t lParam, uint64_t wParam, uint32_t message) -> void {
	uint64_t key_code = wParam;

	int64_t bit_29 = (lParam >> 29) & 1; // 1 == system key (basically ALT key + some key)
	int64_t bit_30 = (lParam >> 30) & 1; // 1 == repeatedly pressed
	int64_t bit_31 = (lParam >> 31) & 1; // 0 == pressed, 1 == released

	bool b_is_system_key = (bit_29 == 1);
	bool b_is_repeated = (bit_30 == 1);
	bool b_is_pressed = (bit_31 == 0);

	auto key_event_type = b_is_pressed ? KeyEvent::TYPE::PRESSED : KeyEvent::TYPE::RELEASED;
	KeyEvent key_event = { key_event_type, key_code };

	std::vector<KeyEvent> key_events;
	key_events.push_back(key_event);

	if (key_event.type == KeyEvent::TYPE::PRESSED) {
		m_current_key_state[key_code] = INPUT_STATE::PRESSED;
	} else if (key_event.type == KeyEvent::TYPE::RELEASED) {
		m_current_key_state[key_code] = INPUT_STATE::RELEASED;
	} else {
		__debugbreak();
	}


	//TODO: Try to extract that to somewhere else.
	if (m_current_key_state[static_cast<int>(KEY::ESCAPE)] == INPUT_STATE::PRESSED) {
		BaseApp::get_instance()->m_is_running = false;
		::PostQuitMessage(0);
	}
}
auto WinInputManager::char_callback(int64_t lParam, uint64_t wParam, uint32_t message) -> void {
	int64_t bit_29 = (lParam >> 29) & 1; // 1 == system key
	int64_t bit_30 = (lParam >> 30) & 1; // 1 == repeatedly pressed
	int64_t bit_31 = (lParam >> 31) & 1; // 0 == pressed, 1 == released


}
auto WinInputManager::is_key_down(const KEY key) const -> bool {
	int key_0 = static_cast<int>(key);
	bool is_current_pressed = (m_current_key_state[key_0] == INPUT_STATE::PRESSED);
	return is_current_pressed ? true : false;
}
auto WinInputManager::is_key_up(const KEY key) const -> bool {
	int key_0 = static_cast<int>(key);
	bool is_current_released = (m_current_key_state[key_0] == INPUT_STATE::RELEASED);
	return is_current_released ? true : false;
}
auto WinInputManager::is_key_pressed(const KEY key) const -> bool {
	int key_0 = static_cast<int>(key);
	bool is_current_changed = (m_current_key_state[key_0] != m_previous_key_state[key_0]);
	bool is_current_pressed = (m_current_key_state[key_0] == INPUT_STATE::PRESSED);
	return (is_current_changed && is_current_pressed) ? true : false;
}
auto WinInputManager::is_key_released(const KEY key) const -> bool {
	int key_0 = static_cast<int>(key);
	bool is_current_changed = (m_current_key_state[key_0] != m_previous_key_state[key_0]);
	bool is_current_released = (m_current_key_state[key_0] == INPUT_STATE::RELEASED);
	return (is_current_changed && is_current_released) ? true : false;
}


/*
	MOUSE INPUT
*/
std::array<INPUT_STATE, 3> WinInputManager::m_current_mouse_button_state = {};
std::array<INPUT_STATE, 3> WinInputManager::m_previous_mouse_button_state = {};

Vec2 WinInputManager::m_mouse_posiition{};
static auto convert_buttons_from_JF_to_imgui(BUTTON button) -> int {
	int result;
	switch (button) {
	case BUTTON::LEFT: result = 0; break;
	case BUTTON::RIGHT: result = 1; break;
	case BUTTON::MIDDLE: result = 4; break;
	default: result = -1; __debugbreak(); break;
	}
	return result;
}
auto WinInputManager::mouse_button_callback(int64_t lParam, int64_t wParam, int32_t message) -> void {

	ImGuiIO& io = ImGui::GetIO();
	BUTTON button;
	switch (message) {
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
	{
		if (message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) {
			button = BUTTON::LEFT;
		}
		if (message == WM_RBUTTONDOWN || message == WM_RBUTTONDBLCLK) {
			button = BUTTON::RIGHT;
		}
		if (message == WM_MBUTTONDOWN || message == WM_MBUTTONDBLCLK) {
			button = BUTTON::MIDDLE;
		}
		m_current_mouse_button_state[static_cast<int>(button)] = INPUT_STATE::PRESSED;
		io.MouseDown[convert_buttons_from_JF_to_imgui(button)] = true;


	}break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	{
		if (message == WM_LBUTTONUP) {
			button = BUTTON::LEFT;
		}
		if (message == WM_RBUTTONUP) {
			button = BUTTON::RIGHT;
		}
		if (message == WM_MBUTTONUP) {
			button = BUTTON::MIDDLE;
		}
		m_current_mouse_button_state[static_cast<int>(button)] = INPUT_STATE::RELEASED;
		io.MouseDown[convert_buttons_from_JF_to_imgui(button)] = false;
	}break;
	}
	int mposx = GET_X_LPARAM(lParam);
	int mposy = GET_Y_LPARAM(lParam);
	m_mouse_posiition.x = mposx;
	m_mouse_posiition.y = mposy;
	io.DeltaTime = 1.0f / 60.0f;
}