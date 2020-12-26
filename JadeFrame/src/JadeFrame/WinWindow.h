#pragma once
#include <string>
#include "math/Vec2.h"
#include <memory>

struct HWND__;
typedef HWND__* HWND;

class WinWindow {
	WinWindow(const std::string& title, Vec2 size, Vec2 position);
public:
	WinWindow() = default;
	static auto init(const std::string& title, Vec2 size, Vec2 position) -> WinWindow;

	auto set_title(const std::string& title);
	//private:

	HWND m_window_handle;
	std::string m_title;
	Vec2 m_size;
	Vec2 m_position;
	bool has_focus = true;
	bool is_minimized = false;
	bool is_maximized = false;
};