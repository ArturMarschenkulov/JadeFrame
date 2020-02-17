#include "Input.h"
#include "Window.h"
#include "BaseApp.h"


void Input::handleInput() {

	double mouseX, mouseY;
	glfwGetCursorPos(BaseApp::getAppInstance()->window.handle, &mouseX, &mouseY);
	this->m_currentMousePosition.x = (float)mouseX;
	this->m_currentMousePosition.y = (float)mouseY;

	for(int i = 0; i < 512; i++) {
		m_previousKeyState[i] = m_currentKeyState[i];
	}
	for(int i = 0; i < 3; i++) {
		m_previousButtonState[i] = m_currentButtonState[i];
	}
}

/*******************************/
/* KEY INPUT */
/*******************************/
std::array<INPUT_ACTION, 512> Input::m_currentKeyState = {};
std::array<INPUT_ACTION, 512> Input::m_previousKeyState = {};

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	m_currentKeyState[key] = (INPUT_ACTION)action;

	if(key == (int)KEY::ESCAPE && action == (int)INPUT_ACTION::PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

bool Input::isKeyDown(KEY key) const {
	bool isDown;
	if(m_currentKeyState[(int)key] == INPUT_ACTION::PRESS) {
		isDown = true;
	} else {
		isDown = false;
	}
	return isDown;
}
bool Input::isKeyUp(KEY key) const {
	bool isUp;
	if(m_currentKeyState[(int)key] == INPUT_ACTION::RELEASE) {
		isUp = true;
	} else {
		isUp = false;
	}
}
bool Input::isKeyPressed(KEY key) const {
	bool isPressed = false;
	if((m_currentKeyState[(int)key] != m_previousKeyState[(int)key]) && (m_currentKeyState[(int)key] == INPUT_ACTION::PRESS)) {
		isPressed = true;
	} else {
		isPressed = false;
	}
	return isPressed;
}
bool Input::isKeyReleased(KEY key) const {
	bool isReleased = false;
	if((m_currentKeyState[(int)key] != m_previousKeyState[(int)key]) && (m_currentKeyState[(int)key] == INPUT_ACTION::RELEASE)) {
		isReleased = true;
	} else {
		isReleased = false;
	}
	return isReleased;
}
/*******************************/
/* MOUSE INPUT */
/*******************************/
std::array<INPUT_ACTION, 3> Input::m_currentButtonState = {};
std::array<INPUT_ACTION, 3> Input::m_previousButtonState = {};
void Input::mouseButtonCallback(GLFWwindow* window, int button, int state, int mods) {
	m_currentButtonState[(int)button] = (INPUT_ACTION)state;
}
bool Input::isButtonDown(MOUSE button) const {
	bool isDown;
	if(m_currentButtonState[(int)button] == INPUT_ACTION::PRESS) {
		isDown = true;
	} else {
		isDown = false;
	}
	return isDown;
}
bool Input::isButtonUp(MOUSE button) const {
	bool isUp;
	if(m_currentButtonState[(int)button] == INPUT_ACTION::RELEASE) {
		isUp = true;
	} else {
		isUp = false;
	}
}
bool Input::isButtonPressed(MOUSE button) const {
	bool isPressed = false;
	if((m_currentButtonState[(int)button] != m_currentButtonState[(int)button]) && (m_currentButtonState[(int)button] == INPUT_ACTION::PRESS)) {
		isPressed = true;
	} else {
		isPressed = false;
	}
	return isPressed;
}
bool Input::isButtonReleased(MOUSE button) const {
	bool isReleased = false;
	if((m_currentButtonState[(int)button] != m_currentButtonState[(int)button]) && (m_currentButtonState[(int)button] == INPUT_ACTION::RELEASE)) {
		isReleased = true;
	} else {
		isReleased = false;
	}
	return isReleased;
}
/*******************************/
/* MOUSE POSITION INPUT */
/*******************************/

Vec2 Input::m_currentMousePosition = { 0 };

void Input::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	m_currentMousePosition = { (float)xpos, (float)ypos };
}
float Input::getMouseX(void) const {
	return m_currentMousePosition.x;
}
float Input::getMouseY(void) const {
	return m_currentMousePosition.y;
}
Vec2 Input::getMousePosition(void) const {
	return m_currentMousePosition;
}
/*******************************/
/* MOUSE ENTER INPUT */
/*******************************/

bool Input::m_isCursorInside = 0;
void Input::cursorEnterCallback(GLFWwindow* window, int entered) {
	switch(entered) {
	case true: m_isCursorInside = true; break;
	case false: m_isCursorInside = false; break;
	}
}

bool Input::isMouseInside() const {
	return m_isCursorInside;
}