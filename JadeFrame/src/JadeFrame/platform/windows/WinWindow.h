#pragma once
#include <windows.h>

#include "../../math/Vec2.h"

#include <string>

struct HWND__;
typedef HWND__* HWND;
struct HGLRC__;
typedef HGLRC__* HGLRC;
struct HDC__;
typedef HDC__* HDC;

struct WindowsMessage {
	HWND hWnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
};

class WinWindow {
public:
	WinWindow(const std::string& title, Vec2 size, Vec2 position);

	WinWindow() = default;
	//static auto init(const std::string& title, Vec2 size, Vec2 position)->WinWindow;
	~WinWindow();
	auto deinit() -> void;
	auto _init(const std::string& title, Vec2 size, Vec2 position) -> void;

	auto set_title(const std::string& title);
	auto set_v_sync(bool b) -> void;
	auto make_current() -> void;
	//private:

	HWND m_window_handle;
	HGLRC m_render_context;
	HDC m_device_context;

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