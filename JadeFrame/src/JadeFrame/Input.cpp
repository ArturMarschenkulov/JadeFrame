#include "Input.h"
#include "BaseApp.h"
#include <iostream>
std::array<bool, 512> InputManager::m_current_key_state = {};
std::array<bool, 512> InputManager::m_previous_key_state = {};
auto InputManager::handle_input() -> void {
	for (int i = 0; i < m_current_key_state.size(); i++) {
		m_previous_key_state[i] = m_current_key_state[i];
	}
}
#include "GUI.h"
#include "Windows.h"
auto InputManager::key_callback(int64_t lParam, uint64_t wParam, uint32_t message) -> void {
	auto keycode = wParam;
	auto bit29 = (lParam >> 29) & 1; // 1 == system key
	auto bit30 = (lParam >> 30) & 1; // 1 == repeatedly pressed
	auto bit31 = (lParam >> 31) & 1; // 0 == pressed, 1 == released
	ImGuiIO& io = ImGui::GetIO();

	bool b_is_system_key = (bit29 == 1);
	bool b_is_repeated = (bit30 == 1);
	bool b_is_pressed = (bit31 == 0);

	io.KeysDown[wParam] = bit31;


	m_current_key_state[keycode] = b_is_pressed;

	std::cout << keycode << ", " << b_is_system_key << ", " << b_is_repeated << ", " << b_is_pressed << std::endl;
	std::cout << m_current_key_state[(int)KEY::ESCAPE] << std::endl;
	if (m_current_key_state[(int)KEY::ESCAPE] == (int)INPUT_STATE::PRESSED)
	{
		BaseApp::get_app_instance()->get_window().set_running(false);
		std::cout << "s" << std::endl;
		PostQuitMessage(0);
	}
}

auto InputManager::is_key_down(const KEY key) const -> bool {
	bool is_down;
	if (m_current_key_state[(int)key] == true) {
		is_down = true;
	} else {
		is_down = false;
	}
	return is_down;
}

auto InputManager::is_key_up(const KEY key) const -> bool {
	bool is_up;
	if (m_current_key_state[(int)key] == false) {
		is_up = true;
	} else {
		is_up = false;
	}
	return is_up;
}

auto InputManager::is_key_pressed(const KEY key) const -> bool {
	bool is_pressed = false;
	if ((m_current_key_state[(int)key] != m_previous_key_state[(int)key]) && (m_current_key_state[(int)key] == true)) {
		is_pressed = true;
	} else {
		is_pressed = false;
	}
	return is_pressed;
}

auto InputManager::is_key_released(const KEY key) const -> bool {
	bool is_released = false;
	if ((m_current_key_state[(int)key] != m_previous_key_state[(int)key]) && (m_current_key_state[(int)key] == false)) {
		is_released = true;
	} else {
		is_released = false;
	}
	return is_released;
}

auto InputManager::mouse_button_callback(int32_t button, int32_t x, int32_t y) -> void {

	auto hWnd = BaseApp::get_app_instance()->get_window().m_window_handle;
	switch(button) {
	case WM_LBUTTONDOWN:
	{
		SetCapture(hWnd);
	}break;
	case WM_LBUTTONUP:
	{
		ReleaseCapture();
	}break;	
	case WM_RBUTTONDOWN:
	{
		SetCapture(hWnd);
	}break;
	case WM_RBUTTONUP:
	{
		ReleaseCapture();
	}break;
	case WM_MBUTTONDOWN:
	{
		SetCapture(hWnd);
	}break;
	case WM_MBUTTONUP:
	{
		ReleaseCapture();
	}break;
	}
}
