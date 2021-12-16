#pragma once

#include "../platform_shared.h"
#include <X11/Xlib.h>


struct HWND__;
typedef HWND__* HWND;
namespace JadeFrame {


class Linux_Window : public IWindow {
public:
	Linux_Window();
	Linux_Window(const Linux_Window::Desc& desc);
	~Linux_Window();
	virtual auto handle_events(bool& running) -> void override {

	}
	virtual auto set_title(const std::string& title) -> void override {

	}
	virtual auto get_title() const -> std::string override {
		return {};
	}
	virtual auto get_window_state() const->WINDOW_STATE override {
		return {};
	}
	virtual auto get_size() const -> const v2u32& override {
		return m_size;
	}

public:
	::Display* m_display;
	::Window m_window;
	HWND m_window_handle = nullptr;
	v2u32 m_size;
};
#ifdef __linux__
using Window = Linux_Window;
#endif
}