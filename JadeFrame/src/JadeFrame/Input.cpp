#include "Input.h"
#include "Window.h"
#include "BaseApp.h"


void Input::handleInput() {

	double mouseX, mouseY;
	glfwGetCursorPos(BaseApp::getAppInstance()->window.handle, &mouseX, &mouseY);
	this->m_current_mouse_position.x = (float)mouseX;
	this->m_current_mouse_position.y = (float)mouseY;

	for(int i = 0; i < 512; i++) {
		m_previous_key_state[i] = m_current_key_state[i];
	}
	for(int i = 0; i < 3; i++) {
		m_previous_button_state[i] = m_current_button_state[i];
	}
}

/*******************************/
/* KEY INPUT */
/*******************************/
std::array<INPUT_ACTION, 512> Input::m_current_key_state = {};
std::array<INPUT_ACTION, 512> Input::m_previous_key_state = {};

void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	m_current_key_state[key] = (INPUT_ACTION)action;

	if(key == (int)KEY::ESCAPE && action == (int)INPUT_ACTION::PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

bool Input::is_key_down(KEY key) const {
	bool isDown;
	if(m_current_key_state[(int)key] == INPUT_ACTION::PRESS) {
		isDown = true;
	} else {
		isDown = false;
	}
	return isDown;
}
bool Input::is_key_up(KEY key) const {
	bool isUp;
	if(m_current_key_state[(int)key] == INPUT_ACTION::RELEASE) {
		isUp = true;
	} else {
		isUp = false;
	}
	return isUp;
}
bool Input::is_key_pressed(KEY key) const {
	bool isPressed = false;
	if((m_current_key_state[(int)key] != m_previous_key_state[(int)key]) && (m_current_key_state[(int)key] == INPUT_ACTION::PRESS)) {
		isPressed = true;
	} else {
		isPressed = false;
	}
	return isPressed;
}
bool Input::is_key_released(KEY key) const {
	bool isReleased = false;
	if((m_current_key_state[(int)key] != m_previous_key_state[(int)key]) && (m_current_key_state[(int)key] == INPUT_ACTION::RELEASE)) {
		isReleased = true;
	} else {
		isReleased = false;
	}
	return isReleased;
}
/*******************************/
/* MOUSE INPUT */
/*******************************/
std::array<INPUT_ACTION, 3> Input::m_current_button_state = {};
std::array<INPUT_ACTION, 3> Input::m_previous_button_state = {};
void Input::mouse_button_callback(GLFWwindow* window, int button, int state, int mods) {
	m_current_button_state[(int)button] = (INPUT_ACTION)state;
}
bool Input::is_button_down(MOUSE button) const {
	bool isDown;
	if(m_current_button_state[(int)button] == INPUT_ACTION::PRESS) {
		isDown = true;
	} else {
		isDown = false;
	}
	return isDown;
}
bool Input::is_button_up(MOUSE button) const {
	bool isUp;
	if(m_current_button_state[(int)button] == INPUT_ACTION::RELEASE) {
		isUp = true;
	} else {
		isUp = false;
	}
	return isUp;
}
bool Input::is_button_pressed(MOUSE button) const {
	bool isPressed = false;
	if((m_current_button_state[(int)button] != m_previous_button_state[(int)button]) && (m_current_button_state[(int)button] == INPUT_ACTION::PRESS)) {
		isPressed = true;
	} else {
		isPressed = false;
	}
	return isPressed;
}
bool Input::is_button_released(MOUSE button) const {
	bool isReleased = false;
	if((m_current_button_state[(int)button] != m_previous_button_state[(int)button]) && (m_current_button_state[(int)button] == INPUT_ACTION::RELEASE)) {
		isReleased = true;
	} else {
		isReleased = false;
	}
	return isReleased;
}
/*******************************/
/* MOUSE POSITION INPUT */
/*******************************/

Vec2 Input::m_current_mouse_position = { 0 };

void Input::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	m_current_mouse_position = { (float)xpos, (float)ypos };
}
float Input::get_mouse_X(void) const {
	return m_current_mouse_position.x;
}
float Input::get_mouse_Y(void) const {
	return m_current_mouse_position.y;
}
Vec2 Input::get_mouse_position(void) const {
	return m_current_mouse_position;
}
/*******************************/
/* MOUSE ENTER INPUT */
/*******************************/

bool Input::m_is_cursor_inside = 0;
void Input::cursor_enter_callback(GLFWwindow* window, int entered) {
	switch(entered) {
	case true: m_is_cursor_inside = true; break;
	case false: m_is_cursor_inside = false; break;
	}
}

bool Input::is_mouse_inside() const {
	return m_is_cursor_inside;
}