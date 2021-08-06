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

namespace JadeFrame {

struct WindowsMessage {
	HWND hWnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
};

/*
	TOOD: Consider whether this class should be RAII or not. Right now it is.
*/
class Windows_Window {
public:
	enum class WINDOW_STATE {
		WINDOWED,
		MINIMIZED,
		MAXIMIZED,
	};
	struct DESC {
		std::string title;
		Vec2 size;
		Vec2 position; // NOTE: -1 means randomly chosen by OS
		//bool is_vsync;
		WINDOW_STATE window_state = WINDOW_STATE::WINDOWED;
	};
	Windows_Window(const Windows_Window&) = delete;
	Windows_Window(Windows_Window&&) = delete;
	auto operator=(const Windows_Window&) -> Windows_Window& = delete;
	auto operator=(Windows_Window&&) -> Windows_Window& = delete;

	Windows_Window() = default;
	Windows_Window(const Windows_Window::DESC& desc);
	~Windows_Window();



	auto set_title(const std::string& title) -> void;
	auto get_title() const -> std::string;

	auto set_size(const Vec2& size) -> void;
	auto get_size() const -> Vec2;

	auto set_position(const Vec2& position) -> void;
	auto get_position() const -> Vec2;

	auto set_window_state(const WINDOW_STATE window_state) -> void;
	auto get_window_state() const -> WINDOW_STATE;

	auto query_client_size() const -> Vec2;



public:
	mutable HWND m_window_handle;
public:
	std::string m_title;
	mutable Vec2 m_size;
	mutable Vec2 m_position;
	mutable bool has_focus = true;

	WINDOW_STATE m_window_state;
};

}