#pragma once

#include "../../math/Vec2.h"

#include <string>

struct HWND__;
typedef HWND__* HWND;
struct HGLRC__;
typedef HGLRC__* HGLRC;
struct HDC__;
typedef HDC__* HDC;

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

class Windows_Window {
public:
	~Windows_Window();
	auto initialize(const std::string& title, const Vec2& size, const Vec2& position) -> void;
	auto deinitialize() const -> void;

	auto set_title(const std::string& title);
	auto make_current() const -> void;

public:
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