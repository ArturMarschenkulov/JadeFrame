#include "Input_backup.h"
#include "Window.h"
#include "BaseApp.h"

//auto Input::handle_input() -> void {
//	double mouse_position_X;
//	double mouse_position_Y;
//	//glfwGetCursorPos(BaseApp::get_app_instance()->get_window().get_handle(), &mouse_position_X, &mouse_position_Y);
//	//this->m_current_mouse_position.x = mouse_position_X;
//	//this->m_current_mouse_position.y = mouse_position_Y;
//
//	for(int i = 0; i < 512; i++) {
//		m_previous_key_state[i] = m_current_key_state[i];
//	}
//	for(int i = 0; i < 3; i++) {
//		m_previous_button_state[i] = m_current_button_state[i];
//	}
//}
//
///*******************************/
///* KEY INPUT */
///*******************************/
//std::array<EInputState, 512> Input::m_current_key_state = {};
//std::array<EInputState, 512> Input::m_previous_key_state = {};
//
//auto Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) -> void {
//	m_current_key_state[key] = (EInputState)action;
//
//	if(key == (int)EKey::ESCAPE && action == (int)EInputState::PRESS) {
//		glfwSetWindowShouldClose(window, GL_TRUE);
//	}
//}
//
//auto Input::is_key_down(EKey key) const -> bool {
//	bool is_down;
//	if(m_current_key_state[(int)key] == EInputState::PRESS || m_current_key_state[(int)key] == EInputState::REPEAT) {
//		is_down = true;
//	} else {
//		is_down = false;
//	}
//	return is_down;
//}
//auto Input::is_key_up(EKey key) const -> bool {
//	bool is_up;
//	if(m_current_key_state[(int)key] == EInputState::RELEASE) {
//		is_up = true;
//	} else {
//		is_up = false;
//	}
//	return is_up;
//}
//auto Input::is_key_pressed(EKey key) const -> bool {
//	bool is_pressed = false;
//	if((m_current_key_state[(int)key] != m_previous_key_state[(int)key]) && (m_current_key_state[(int)key] == EInputState::PRESS)) {
//		is_pressed = true;
//	} else {
//		is_pressed = false;
//	}
//	return is_pressed;
//}
//auto Input::is_key_released(EKey key) const -> bool {
//	bool is_released = false;
//	if((m_current_key_state[(int)key] != m_previous_key_state[(int)key]) && (m_current_key_state[(int)key] == EInputState::RELEASE)) {
//		is_released = true;
//	} else {
//		is_released = false;
//	}
//	return is_released;
//}
///*******************************/
///* MOUSE INPUT */
///*******************************/
//std::array<EInputState, 3> Input::m_current_button_state = {};
//std::array<EInputState, 3> Input::m_previous_button_state = {};
//auto Input::mouse_button_callback(GLFWwindow* window, int button, int state, int mods) -> void {
//	m_current_button_state[(int)button] = (EInputState)state;
//}
//auto Input::is_button_down(EMouse button) const -> bool {
//	bool is_down;
//	if(m_current_button_state[(int)button] == EInputState::PRESS || m_current_button_state[(int)button] == EInputState::REPEAT) {
//		is_down = true;
//	} else {
//		is_down = false;
//	}
//	return is_down;
//}
//auto Input::is_button_up(EMouse button) const -> bool {
//	bool is_up;
//	if(m_current_button_state[(int)button] == EInputState::RELEASE) {
//		is_up = true;
//	} else {
//		is_up = false;
//	}
//	return is_up;
//}
//auto Input::is_button_pressed(EMouse button) const -> bool {
//	bool is_pressed = false;
//	if((m_current_button_state[(int)button] != m_previous_button_state[(int)button]) && (m_current_button_state[(int)button] == EInputState::PRESS)) {
//		is_pressed = true;
//	} else {
//		is_pressed = false;
//	}
//	return is_pressed;
//}
//auto Input::is_button_released(EMouse button) const -> bool {
//	bool is_released = false;
//	if((m_current_button_state[(int)button] != m_previous_button_state[(int)button]) && (m_current_button_state[(int)button] == EInputState::RELEASE)) {
//		is_released = true;
//	} else {
//		is_released = false;
//	}
//	return is_released;
//}
///*******************************/
///* MOUSE POSITION INPUT */
///*******************************/
//
//Vec2 Input::m_current_mouse_position = { 0 };
//
//auto Input::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) -> void {
//	m_current_mouse_position = { (float)xpos, (float)ypos };
//}
//auto Input::get_mouse_X(void) const -> float {
//	return m_current_mouse_position.x;
//}
//auto Input::get_mouse_Y(void) const -> float {
//	return m_current_mouse_position.y;
//}
//auto Input::get_mouse_position(void) const -> Vec2 {
//	return m_current_mouse_position;
//}
///*******************************/
///* MOUSE ENTER INPUT */
///*******************************/
//
//bool Input::m_is_cursor_inside = 0;
//auto Input::cursor_enter_callback(GLFWwindow* window, int entered) -> void{
//	switch(entered) {
//	case true: m_is_cursor_inside = true; break;
//	case false: m_is_cursor_inside = false; break;
//	}
//}
//
//auto Input::is_mouse_inside() const -> bool {
//	return m_is_cursor_inside;
//}