#pragma once
#include "Window.h"
#include "graphics/BatchRenderer.h"
#include "graphics/BaseRenderer.h"
#include "Input.h"
class TimeManager {
public:
	void handle_time();
private:
	double currentTime = 0.0;
	double previousTime = 0.0;
	double drawTime = 0.0;
	double frameTime = 0.0;
	double updateTime = 0.0;
	double targetTime = 0.0;
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