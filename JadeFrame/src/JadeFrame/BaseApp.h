#pragma once
#include "Window.h"
#include "BatchRenderer.h"
#include "Input.h"


class BaseApp {
public:

	BaseApp();
	virtual ~BaseApp();

	virtual void setup() {}
	virtual void update() {}
	virtual void draw() {}

	void initApp(const std::string& title, float width, float height);
	void runApp();
	void pollEvents();

	static BaseApp* getAppInstance() {
		return instance;
	}
//private:
	static BaseApp* instance;

//private:
	Window window;
	Input input;
	BatchShader shader;
	BatchRenderer renderer;

};