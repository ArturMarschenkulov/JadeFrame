#pragma once
#include "Window.h"
#include "graphics/BatchRenderer.h"
#include "graphics/BaseRenderer.h"
#include "Input.h"
class TimeManager {
public:
	auto handle_time() -> void;
private:
	double current_time = 0.0;
	double previous_time = 0.0;
	double draw_time = 0.0;
	double frame_time = 0.0;
	double update_time = 0.0;
	double target_time = 0.0;
};

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
	auto get_input() const -> Input { return m_input; };
	//BatchShader& get_shader() { return m_shader; };
	//Renderer* get_renderer() { return m_renderer; };
	Shader m_shader;
	BatchRenderer m_renderer;
private:
	Window m_window;
	Input m_input;



	TimeManager m_time_manager;

};