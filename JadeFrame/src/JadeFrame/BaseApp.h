#pragma once
#include "WinWindow.h"
#include "graphics/GLRenderer.h"
#include "graphics/BaseRenderer.h"
#include "Input.h"




class BaseApp {
public:

	BaseApp();
	virtual ~BaseApp();

	virtual auto setup() -> void {}
	virtual auto update() -> void {}
	virtual auto draw() -> void {}
	auto start(const std::string& title, Vec2 size) -> void;
	auto init(const std::string& title, Vec2 size) -> void;
	auto run() -> void;
	auto poll_events() -> void;
	auto clear(GLbitfield bitfield) -> void;

	static BaseApp* get_app_instance() {
		return instance;
	}
private:
	static BaseApp* instance;
public:
	//private:
	WinWindow m_window;
	InputManager m_input_manager;
};