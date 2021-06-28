#include<JadeFrame.h>
#include<string>
#include "example_0.h"
//#include "example_empty.h"
using namespace JadeFrame;
class App : public BaseApp {
public:
public:
	App(const std::string& title, const Vec2 pos)
		:BaseApp(title, pos) {
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
	std::string title = "Test";
	float width = 1280;
	float height = 720;
	GApp* app = new GApp(title, { width, height });

	jade_frame.add(app);
	jade_frame.run();
	return 0;
#endif
}