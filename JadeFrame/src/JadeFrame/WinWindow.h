#pragma once
#include "math/Vec2.h"

#include <string>

struct HWND__;
typedef HWND__* HWND;

class WinWindow {
	WinWindow(const std::string& title, Vec2 size, Vec2 position);
public:
	WinWindow() = default;
	static auto init(const std::string& title, Vec2 size, Vec2 position)->WinWindow;

	auto set_title(const std::string& title);
	auto set_v_sync(bool b) -> void;
	//private:

	HWND m_window_handle;
	std::string m_title;
	Vec2 m_size;
	Vec2 m_position;
	bool has_focus = true;
	enum class WINDOW_STATE {
		WINDOWED,
		MINIMIZED,
		MAXIMIZED,
	} m_window_state;
};