#pragma once
#include "../extern/glad/glad.h"
#include <array>
#include "math/Vec2.h"


enum class EInputState {
	PRESSED,
	RELEASED
};

enum class EKey {
	SPACE = 32,
	ESCAPE = 256,
	ENTER = 257,
	TAB = 258,
	BACKSPACE = 259,
	INSERT = 260,
	DELET = 261,
	RIGHT = 262,
	LEFT = 263,
	DOWN = 264,
	UP = 265,
	PAGE_UP = 266,
	PAGE_DOWN = 267,
	HOME = 268,
	END = 269,
	CAPS_LOCK = 280,
	SCROLL_LOCK = 281,
	NUM_LOCK = 282,
	PRINT_SCREEN = 283,
	PAUSE = 284,
	F1 = 290,
	F2 = 291,
	F3 = 292,
	F4 = 293,
	F5 = 294,
	F6 = 295,
	F7 = 296,
	F8 = 297,
	F9 = 298,
	F10 = 299,
	F11 = 300,
	F12 = 301,
	LEFT_SHIFT = 340,
	LEFT_CONTROL = 341,
	LEFT_ALT = 342,
	RIGHT_SHIFT = 344,
	RIGHT_CONTROL = 345,
	RIGHT_ALT = 346,
	GRAVE = 96,
	SLASH = 47,
	BACKSLASH = 92,

	ZERO = 48,
	ONE = 49,
	TWO = 50,
	THREE = 51,
	FOUR = 52,
	FIVE = 53,
	SIX = 54,
	SEVEN = 55,
	EIGHT = 56,
	NINE = 57,
	A = 65,
	B = 66,
	C = 67,
	D = 68,
	E = 69,
	F = 70,
	G = 71,
	H = 72,
	I = 73,
	J = 74,
	K = 75,
	L = 76,
	M = 77,
	N = 78,
	O = 79,
	P = 80,
	Q = 81,
	R = 82,
	S = 83,
	T = 84,
	U = 85,
	V = 86,
	W = 87,
	X = 88,
	Y = 89,
	Z = 90,
};
class InputManager {
	friend class Window;
public:
	auto handle_input() -> void;
	static auto key_callback(int64_t lParam, uint64_t wParam, uint32_t message) -> void;
	static std::array<bool, 512> m_current_key_state;
	static std::array<bool, 512> m_previous_key_state;

	auto is_key_down(const EKey key) -> bool;
	auto is_key_up(const EKey key) const -> bool;
	auto is_key_pressed(const EKey key) const -> bool;
	auto is_key_released(const EKey key) const -> bool;
};