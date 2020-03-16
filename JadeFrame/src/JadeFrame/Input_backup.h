#pragma once
//#include "../extern/glad/glad.h"
//#include "../extern/GLFW/glfw3.h"
////#include "../extern/glm/glm.hpp"
//#include <array>
//#include "math/Vec2.h"
//
//
//
//enum class EInputState {
//	PRESS = GLFW_PRESS,
//	RELEASE = GLFW_RELEASE,
//	REPEAT = GLFW_REPEAT,
//	UNKNOWN = GLFW_KEY_UNKNOWN
//};
//enum class EKey {
//	SPACE = 32,
//	ESCAPE = 256,
//	ENTER = 257,
//	TAB = 258,
//	BACKSPACE = 259,
//	INSERT = 260,
//	DELET = 261,
//	RIGHT = 262,
//	LEFT = 263,
//	DOWN = 264,
//	UP = 265,
//	PAGE_UP = 266,
//	PAGE_DOWN = 267,
//	HOME = 268,
//	END = 269,
//	CAPS_LOCK = 280,
//	SCROLL_LOCK = 281,
//	NUM_LOCK = 282,
//	PRINT_SCREEN = 283,
//	PAUSE = 284,
//	F1 = 290,
//	F2 = 291,
//	F3 = 292,
//	F4 = 293,
//	F5 = 294,
//	F6 = 295,
//	F7 = 296,
//	F8 = 297,
//	F9 = 298,
//	F10 = 299,
//	F11 = 300,
//	F12 = 301,
//	LEFT_SHIFT = 340,
//	LEFT_CONTROL = 341,
//	LEFT_ALT = 342,
//	RIGHT_SHIFT = 344,
//	RIGHT_CONTROL = 345,
//	RIGHT_ALT = 346,
//	GRAVE = 96,
//	SLASH = 47,
//	BACKSLASH = 92,
//
//	ZERO = 48,
//	ONE = 49,
//	TWO = 50,
//	THREE = 51,
//	FOUR = 52,
//	FIVE = 53,
//	SIX = 54,
//	SEVEN = 55,
//	EIGHT = 56,
//	NINE = 57,
//	A = 65,
//	B = 66,
//	C = 67,
//	D = 68,
//	E = 69,
//	F = 70,
//	G = 71,
//	H = 72,
//	I = 73,
//	J = 74,
//	K = 75,
//	L = 76,
//	M = 77,
//	N = 78,
//	O = 79,
//	P = 80,
//	Q = 81,
//	R = 82,
//	S = 83,
//	T = 84,
//	U = 85,
//	V = 86,
//	W = 87,
//	X = 88,
//	Y = 89,
//	Z = 90,
//};
//enum class EMouse {
//	LEFT_BUTTON = 0,
//	RIGHT_BUTTON = 1,
//	MIDDLE_BUTTON = 2,
//};
//
//
//class Input_ {
//	friend class Window;
//	friend class MainLoop;
//public:
//	Input_()
//		: m_window_instance(nullptr) {
//	}
//	~Input_() {}
//
//	auto handle_input() -> void;
//
//	auto setWindowInstance(GLFWwindow* window) -> void { m_window_instance = window; }
//	auto getWindowInstance() const -> GLFWwindow* { return m_window_instance; }
//
//private:
//	GLFWwindow* m_window_instance;
//
//	// Key Input
//public:
//	auto is_key_down(EKey key) const -> bool;
//	auto is_key_up(EKey key) const -> bool;
//	auto is_key_pressed(EKey key) const -> bool;
//	auto is_key_released(EKey key) const -> bool;
//
//	auto get_down_key(void) const -> char;
//	auto get_up_key(void) const -> char;
//	auto get_pressed_key(void) const -> char;
//	auto get_released_key(void) const -> char;
//
//	//private:
//	static auto key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) -> void;
//	static std::array<EInputState, 512> m_current_key_state;
//	static std::array<EInputState, 512> m_previous_key_state;
//
//
//
//	// Mouse Input
//public:
//	auto is_button_down(const EMouse key) const -> bool;
//	auto is_button_up(const EMouse key) const -> bool;
//	auto is_button_pressed(const EMouse key) const -> bool;
//	auto is_button_released(const EMouse key) const -> bool;
//
//	auto get_down_button() const -> char;
//	auto get_up_button() const -> char;
//	auto get_pressed_button() const -> char;
//	auto get_released_button() const -> char;
//
//	//private:
//	static auto mouse_button_callback(GLFWwindow* window, int button, int state, int mods) -> void;
//	static std::array<EInputState, 3> m_current_button_state;
//	static std::array<EInputState, 3> m_previous_button_state;
//
//	// Mouse Position
//public:
//	auto get_mouse_X() const -> float;
//	auto get_mouse_Y() const -> float;
//	auto get_mouse_position() const->Vec2;
//	//private:
//	static auto cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) -> void;
//	static Vec2 m_current_mouse_position;
//
//	// Mouse Enter
//public:
//	auto is_mouse_inside() const -> bool;
//	//private:
//	static auto cursor_enter_callback(GLFWwindow* window, int entered) -> void;
//	static bool m_is_cursor_inside;
//};
