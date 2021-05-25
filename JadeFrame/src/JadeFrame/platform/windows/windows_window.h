#pragma once

#include "JadeFrame/math/vec_2.h"

#include <string>

struct HWND__;
typedef HWND__* HWND;

typedef unsigned int        UINT;
typedef unsigned __int64	UINT_PTR;
typedef UINT_PTR            WPARAM;
typedef __int64				LONG_PTR;
typedef LONG_PTR            LPARAM;

struct WindowsMessage {
	HWND hWnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
};

/*
	TODO: Consider whether this class should be RAII or not. Right now it is.
*/
class Windows_Window {
public:

	Windows_Window(const Windows_Window&) = delete;
	Windows_Window(Windows_Window&&) = delete;
	auto operator=(const Windows_Window&) -> Windows_Window& = delete ;
	auto operator=(Windows_Window&&) -> Windows_Window& = delete;

	Windows_Window();
	Windows_Window(const std::string& title, const Vec2& size, const Vec2& position);
	~Windows_Window();


	auto set_title(const std::string& title) -> void;

public:
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