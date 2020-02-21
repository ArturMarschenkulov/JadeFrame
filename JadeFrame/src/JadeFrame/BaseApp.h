#pragma once
#include "Window.h"
#include "graphics/BatchRenderer.h"
#include "graphics/BaseRenderer.h"
#include "Input.h"
class TimeManager {
public:
	void handle_time();
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

	virtual void setup() {}
	virtual void update() {}
	virtual void draw() {}

	void init_app(const std::string& title, float width, float height);
	void run_app();
	void poll_events();

	static BaseApp* get_app_instance() {
		return instance;
	}
//private:
	static BaseApp* instance;

//private:
	Window window;
	Input input;
	BatchShader shader;
	BatchRenderer renderer;

	TimeManager time_manager;

};