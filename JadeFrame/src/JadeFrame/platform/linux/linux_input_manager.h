#pragma once
#include "../platform_shared.h"

namespace JadeFrame {
// enum class INPUT_STATE {
// 	RELEASED,
// 	PRESSED
// };

// enum class BUTTON {
// 	LEFT,
// 	RIGHT,
// 	MIDDLE,
// 	X1,
// 	X2,
// };
// enum class KEY {
// 	SPACE,
// 	ESCAPE,// 256,
// 	ENTER,
// 	TAB,
// 	BACKSPACE,
// 	INSERT,
// 	DELET,
// 	RIGHT,
// 	LEFT,
// 	DOWN,
// 	UP,
// 	PAGE_UP,
// 	PAGE_DOWN,
// 	HOME,
// 	END,
// 	CAPS_LOCK,
// 	SCROLL_LOCK,
// 	NUM_LOCK,
// 	PRINT_SCREEN,
// 	PAUSE,
// 	F1,
// 	F2,
// 	F3,
// 	F4,
// 	F5,
// 	F6,
// 	F7,
// 	F8,
// 	F9,
// 	F10,
// 	F11,
// 	F12,
// 	LEFT_SHIFT,
// 	LEFT_CONTROL,
// 	LEFT_ALT,
// 	RIGHT_SHIFT,
// 	RIGHT_CONTROL,
// 	RIGHT_ALT,
// 	GRAVE,
// 	SLASH,
// 	BACKSLASH,

// 	ZERO = 48,
// 	ONE = 49,
// 	TWO = 50,
// 	THREE = 51,
// 	FOUR = 52,
// 	FIVE = 53,
// 	SIX = 54,
// 	SEVEN = 55,
// 	EIGHT = 56,
// 	NINE = 57,
// 	A = 65,
// 	B = 66,
// 	C = 67,
// 	D = 68,
// 	E = 69,
// 	F = 70,
// 	G = 71,
// 	H = 72,
// 	I = 73,
// 	J = 74,
// 	K = 75,
// 	L = 76,
// 	M = 77,
// 	N = 78,
// 	O = 79,
// 	P = 80,
// 	Q = 81,
// 	R = 82,
// 	S = 83,
// 	T = 84,
// 	U = 85,
// 	V = 86,
// 	W = 87,
// 	X = 88,
// 	Y = 89,
// 	Z = 90,
// };
class Linux_InputManager : public IInputManager {
public:
	virtual auto handle_input() -> void override {
		return;
	}
    static auto is_key_down(const KEY key) -> bool {
		return {};
	}
    static auto is_key_up(const KEY key) -> bool {
		return {};
	}
    static auto is_key_pressed(const KEY key) -> bool {
		return {};
	}
    static auto is_key_released(const KEY key) -> bool {
		return {};
	}
};
#ifdef __linux__
using InputManager = Linux_InputManager;
#endif

}