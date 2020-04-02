#pragma once
#include "Window.h"
#include "graphics/BatchRenderer.h"
#include "graphics/BaseRenderer.h"
#include "Input.h"


class BaseApp {
public:

	BaseApp();
	virtual ~BaseApp();

	virtual auto setup() -> void {}
	virtual auto update() -> void {}
	virtual auto draw() -> void {}

	auto init_app(const std::string& title, Vec2 size) -> void;
	auto run_app() -> void;
	auto poll_events() -> void;

	static BaseApp* get_app_instance() {
		return instance;
	}
private:
	static BaseApp* instance;
public:
	auto get_window() const -> Window { return m_window; };
	//auto get_input() const -> Input { return m_input; };
	//BatchShader& get_shader() { return m_shader; };
	//Renderer* get_renderer() { return m_renderer; };
	Shader m_shader;
	BatchRenderer m_renderer;
	//private:
	Window m_window;
	InputManager m_input_manager;
	//Input m_input;
	Camera cam;
};