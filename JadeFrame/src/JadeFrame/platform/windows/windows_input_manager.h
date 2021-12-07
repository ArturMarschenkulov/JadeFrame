#pragma once
#include "JadeFrame/defines.h"
#include "JadeFrame/math/vec.h"
#include "../platform_shared.h"
#include <Windows.h>

#include <array>
#include <queue>

namespace JadeFrame {

struct WindowsMessage;
enum class INPUT_STATE {
	RELEASED,
	PRESSED
};

enum class BUTTON {
	LEFT = VK_LBUTTON,
	RIGHT = VK_RBUTTON,
	MIDDLE = VK_MBUTTON,
	X1 = VK_XBUTTON1,
	X2 = VK_XBUTTON2,
};
enum class KEY {
	SPACE = VK_SPACE,
	ESCAPE = VK_ESCAPE,// 256,
	ENTER = VK_RETURN,
	TAB = VK_TAB,
	BACKSPACE = VK_BACK,
	INSERT = VK_INSERT,
	DELET = VK_DELETE,
	RIGHT = VK_RIGHT,
	LEFT = VK_LEFT,
	DOWN = VK_DOWN,
	UP = VK_UP,
	PAGE_UP = VK_PRIOR,
	PAGE_DOWN = VK_NEXT,
	HOME = VK_HOME,
	END = VK_END,
	CAPS_LOCK = VK_CAPITAL,
	SCROLL_LOCK = VK_SCROLL,
	NUM_LOCK = VK_NUMLOCK,
	PRINT_SCREEN = VK_SNAPSHOT,
	PAUSE = VK_PAUSE,
	F1 = VK_F1,
	F2 = VK_F2,
	F3 = VK_F3,
	F4 = VK_F4,
	F5 = VK_F5,
	F6 = VK_F6,
	F7 = VK_F7,
	F8 = VK_F8,
	F9 = VK_F9,
	F10 = VK_F10,
	F11 = VK_F11,
	F12 = VK_F12,
	LEFT_SHIFT = VK_LSHIFT,
	LEFT_CONTROL = VK_LCONTROL,
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
struct Event {

};
struct KeyEvent : public Event {
	enum class TYPE {
		PRESSED,
		RELEASED
	};
	KeyEvent(const TYPE type, const u64 key_code)
		: type(type)
		, key_code(key_code) {

	}
	TYPE type;
	u64 key_code;
};
class Windows_InputManager : public IInputManager {
	friend class WinWindow;
public:
	auto handle_input() -> void;

public:
	//key part
	std::queue<KeyEvent> key_buffer;
	//static auto key_callback(i64 lParam, u64 wParam, u32 message) -> void;
	static auto key_callback(const WindowsMessage& window_message) -> void;
	static auto key_callback2(i64 lParam, u64 wParam, u32 message) -> void;
	static auto char_callback(const WindowsMessage& window_message) -> void;

	static auto is_key_down(const KEY key) -> bool;
	static auto is_key_up(const KEY key) -> bool;
	static auto is_key_pressed(const KEY key) -> bool;
	static auto is_key_released(const KEY key) -> bool;

	static std::array<INPUT_STATE, 512> m_current_key_state;
	static std::array<INPUT_STATE, 512> m_previous_key_state;

public:
	//mouse part
	static auto mouse_button_callback(const WindowsMessage& window_message) -> void;

	auto is_button_down(const BUTTON button) const -> bool;
	auto is_button_up(const BUTTON button) const -> bool;
	auto is_button_pressed(const BUTTON button) const -> bool;
	auto is_button_released(const BUTTON button) const -> bool;
	auto get_mouse_position() const->v2;

	static std::array<INPUT_STATE, 3> m_current_mouse_button_state;
	static std::array<INPUT_STATE, 3> m_previous_mouse_button_state;
	static v2 m_mouse_posiition;
};

}