#pragma once
#include <string>
#include "math/Vec2.h"

struct HWND__;
typedef HWND__* HWND;

class WinWindow {
public:
	WinWindow();

	auto init(const std::string& title, Vec2 size) -> void;

	//auto get_handle() const -> GLFWwindow*;
	auto get_size() const -> Vec2;
	auto get_width() const -> float;
	auto get_height() const -> float;
	//private:
	HWND m_window_handle;
	Vec2 m_size;
	std::string m_title;

	auto set_running(bool is_running) -> void { m_is_running = is_running; }
	bool m_is_running = true;



};