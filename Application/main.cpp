#include<JadeFrame.h>

//#include "example_0.h"
//#include "example_empty.h"
//#include "exmaple_rotating_primitive.h"
#include "example_texture_quad.h"

#include <string>
#include <iostream>
using namespace JadeFrame;
class App : public BaseApp {
public:
public:
	App(DESC desc)
		: BaseApp(desc) {
	}
	virtual auto on_init() -> void override {
	}
	virtual auto on_update() -> void override {
	}
	virtual auto on_draw() -> void override {
	}
};


int main() {

#if 1
	using GApp = TestApp;
	JadeFrameInstance jade_frame;

	GApp::DESC win_desc;
	win_desc.title = "Test";
	win_desc.size.width = 800;// = 1280;
	win_desc.size.height = 800;// = 720;

	GApp* app = new GApp(win_desc);

	jade_frame.add(app);
	jade_frame.run();
	return 0;
#endif
}



