#include "Input.h"
#include "BaseApp.h"
#include <iostream>

std::array<INPUT_STATE, 512> InputManager::m_current_key_state = {};
std::array<INPUT_STATE, 512> InputManager::m_previous_key_state = {};

auto InputManager::handle_input() -> void {
	for (int i = 0; i < m_current_key_state.size(); i++) {
		m_previous_key_state[i] = m_current_key_state[i];
	}
}
#include "GUI.h"
#include "Windows.h"
#include <windowsx.h>
auto InputManager::key_callback(int64_t lParam, uint64_t wParam, uint32_t message) -> void {
	auto keycode = wParam;
	auto bit29 = (lParam >> 29) & 1; // 1 == system key
	auto bit30 = (lParam >> 30) & 1; // 1 == repeatedly pressed
	auto bit31 = (lParam >> 31) & 1; // 0 == pressed, 1 == released

	//bool b_is_system_key = (bit29 == 1);
	//bool b_is_repeated = (bit30 == 1);
	bool b_is_pressed = (bit31 == 0);

	m_current_key_state[keycode] = static_cast<INPUT_STATE>(b_is_pressed);

	if (m_current_key_state[static_cast<int>(KEY::ESCAPE)] == INPUT_STATE::PRESSED) {
		BaseApp::get_app_instance()->m_window.m_is_running = false;
		::PostQuitMessage(0);
	}
}

auto InputManager::is_key_down(const KEY key) const -> bool {
	bool is_down;
	if (m_current_key_state[static_cast<int>(key)] == INPUT_STATE::PRESSED) {
		is_down = true;
	} else {
		is_down = false;
	}
	return is_down;
}
auto InputManager::is_key_up(const KEY key) const -> bool {
	bool is_up;
	if (m_current_key_state[static_cast<int>(key)] == INPUT_STATE::RELEASED) {
		is_up = true;
	} else {
		is_up = false;
	}
	return is_up;
}
auto InputManager::is_key_pressed(const KEY key) const -> bool {
	bool is_pressed = false;
	if ((m_current_key_state[static_cast<int>(key)] != m_previous_key_state[static_cast<int>(key)]) && (m_current_key_state[static_cast<int>(key)] == INPUT_STATE::PRESSED)) {
		is_pressed = true;
	} else {
		is_pressed = false;
	}
	return is_pressed;
}
auto InputManager::is_key_released(const KEY key) const -> bool {
	bool is_released = false;
	if ((m_current_key_state[static_cast<int>(key)] != m_previous_key_state[static_cast<int>(key)]) && (m_current_key_state[static_cast<int>(key)] == INPUT_STATE::RELEASED)) {
		is_released = true;
	} else {
		is_released = false;
	}
	return is_released;
}

std::array<INPUT_STATE, 3> InputManager::m_current_mouse_button_state = {};
std::array<INPUT_STATE, 3> InputManager::m_previous_mouse_button_state = {};

Vec2 InputManager::m_mouse_posiition{};
static auto convert_buttons_from_JF_to_imgui(BUTTON button) -> int {
	int result;
	switch (button) {
		case BUTTON::LEFT  : result = 0; break;
		case BUTTON::RIGHT : result = 1; break;
		case BUTTON::MIDDLE: result = 4; break;
		default: result = -1; __debugbreak(); break;
	}
	return result;
}
auto InputManager::mouse_button_callback(int64_t lParam, int64_t wParam, int32_t message) -> void {

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
	auto mposx = GET_X_LPARAM(lParam);
	auto mposy = GET_Y_LPARAM(lParam);
	m_mouse_posiition.x = mposx;
	m_mouse_posiition.y = mposy;
	io.DeltaTime = 1.0f / 60.0f;
}
