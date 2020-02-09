#include "Input.h"
#include "Window.h"
#include "BaseApp.h"

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

	auto state = glfwGetKey(BaseApp::getAppInstance()->window.handle, static_cast<int>(key));
	if(m_currentKeyState[(int)key] == INPUT_ACTION::PRESS ||
	   m_currentKeyState[(int)key] == INPUT_ACTION::REPEAT) {
		//std::cout << static_cast<int>(key) << " is1 " << (int)m_currentKeyState[static_cast<int>(key)] << std::endl;
		return true;
	} else {
		//std::cout << static_cast<int>(key) << " is2 " << (int)m_currentKeyState[static_cast<int>(key)] << std::endl;
		return false;
	}
}

bool Input::isKeyUp(KEY key) const {
	if(m_currentKeyState[(int)key] == INPUT_ACTION::RELEASE) {
		//std::cout << key << " is up" << std::endl;
		return true;
	} else {
		return false;
	}
}
bool Input::isKeyPressed(KEY key) const {
	static bool once = false;

	if(m_currentKeyState[(int)key] == INPUT_ACTION::PRESS) {
		if(once == false) {
			once = true;
			return true;
		}
	}
	if(once == true) {
		if(m_currentKeyState[(int)key] == INPUT_ACTION::RELEASE) {
			once = false;
			return false;
		}
	}
	return false;
}
bool Input::isKeyReleased(KEY key) const {
	static bool once = false;


	static bool wasPressed = false;
	if(wasPressed == false) {
		if(m_currentKeyState[(int)key] == INPUT_ACTION::PRESS) {
			wasPressed = true;
		}
	}

	if(once == true) {
		if(m_currentKeyState[(int)key] == INPUT_ACTION::PRESS) {
			once = false;
		}
	}

	if(m_currentKeyState[(int)key] == INPUT_ACTION::RELEASE) {
		if(wasPressed == true) {
			if(once == false) {
				once = true;
				wasPressed = false;
				return true;

			}
		}
	}
	return false;
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
	if(m_currentButtonState[(int)button] == INPUT_ACTION::PRESS) {
		m_previousButtonState[(int)button] = m_currentButtonState[(int)button];
		return true;
	} else {
		m_previousButtonState[(int)button] = m_currentButtonState[(int)button];
		return false;
	}
}
bool Input::isButtonUp(MOUSE button) const {
	if(m_currentButtonState[(int)button] == INPUT_ACTION::RELEASE) {
		//std::cout << button << " is up" << std::endl;
		m_previousButtonState[(int)button] = m_currentButtonState[(int)button];
		return true;
	} else {
		m_previousButtonState[(int)button] = m_currentButtonState[(int)button];
		return false;
	}
}
bool Input::isButtonPressed(MOUSE button) const {
	if(m_previousButtonState[(int)button] == INPUT_ACTION::RELEASE) {
		if(m_currentButtonState[(int)button] == INPUT_ACTION::PRESS) {
			m_previousButtonState[(int)button] = m_currentButtonState[(int)button];
			return true;
		} else {
			return false;
		}
	} else {
		m_previousButtonState[(int)button] = m_currentButtonState[(int)button];
		return false;
	}
}
bool Input::isButtonReleased(MOUSE button) const {
	if(m_currentButtonState[(int)button] == INPUT_ACTION::RELEASE) {
		if(m_previousButtonState[(int)button] == INPUT_ACTION::PRESS) {
			m_previousButtonState[(int)button] = m_currentButtonState[(int)button];
			return true;
		} else {
			return false;
		}
	} else {
		m_previousButtonState[(int)button] = m_currentButtonState[(int)button];
		return false;
	}
}
/*******************************/
/* MOUSE POSITION INPUT */
/*******************************/

Vec2 Input::m_currentMousePosition = { 0 };

void Input::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	m_currentMousePosition = { (float)xpos, (float)ypos };
}

float Input::getMouseX(void) const {

	double mouseX;
	glfwGetCursorPos(BaseApp::getAppInstance()->window.handle, &mouseX, nullptr);
	this->m_currentMousePosition.x = (float)mouseX;
	return m_currentMousePosition.x;

	//return m_currentMousePosition.x;
}
float Input::getMouseY(void) const {

	double mouseY;
	glfwGetCursorPos(BaseApp::getAppInstance()->window.handle, nullptr, &mouseY);
	this->m_currentMousePosition.y = (float)mouseY;
	return m_currentMousePosition.y;

	//return m_currentMousePosition.y;
}
Vec2 Input::getMousePosition(void) const {
	double mouseX, mouseY;
	glfwGetCursorPos(BaseApp::getAppInstance()->window.handle, &mouseX, &mouseY);
	this->m_currentMousePosition.x = (float)mouseX;
	this->m_currentMousePosition.y = (float)mouseY;
	return m_currentMousePosition;

	//return m_currentMousePosition;
}
/*******************************/
/* MOUSE ENTER INPUT */
/*******************************/

bool Input::m_isCursorInside = 0;

bool Input::isMouseInside() const {
	return m_isCursorInside;
}

void Input::cursorEnterCallback(GLFWwindow* window, int entered) {
	switch(entered) {
	case true: m_isCursorInside = true; break;
	case false: m_isCursorInside = false; break;
	}
}
