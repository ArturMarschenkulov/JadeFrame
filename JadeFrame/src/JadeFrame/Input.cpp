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
auto InputManager::key_callback(int64_t lParam, uint64_t wParam, uint32_t message) -> void {
	auto keycode = wParam;
	auto bit29 = (lParam >> 29) & 1; // 1 == system key
	auto bit30 = (lParam >> 30) & 1; // 1 == repeatedly pressed
	auto bit31 = (lParam >> 31) & 1; // 0 == pressed, 1 == released


	bool is_system_key = (bit29 == 1);
	bool is_repeated = (bit30 == 1);
	bool is_pressed = (bit31 == 0);


	m_current_key_state[keycode] = is_pressed;

	if (m_current_key_state[(int)EKey::ESCAPE] == is_pressed) {

		BaseApp::get_app_instance()->get_window().set_running(false);
	}


	//auto is_pressed = (b31 == 0) && (b30 == 0);
	//auto is_released = (b31 == 1) && (b30 == 1);
	//if (is_released) {
	//	std::cout << (char)keycode << " " << b29 << b30 << b31 << std::endl;
	//}
	//int key, scancode;
	//const int action = (HIWORD(lParam) & KF_UP) ? GLFW_RELEASE : GLFW_PRESS;
	//const int mods = getKeyMods();
	//if (wParam == VK_ESCAPE) {
	//	PostQuitMessage(0);
	//}

}
