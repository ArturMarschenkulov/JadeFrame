#include<JadeFrame.h>
#include<iostream>
#include <variant>
#include <map>
#include <cassert>
#include <JadeFrame/uilts/Utils.h>

class App : public BaseApp {
public:
public:
	virtual auto init() -> void override {
		std::cout << "sss" << std::endl;
	}
	virtual auto update() -> void override {
		std::cout << "sss" << std::endl;
	}
	virtual auto draw() -> void override {
		std::cout << "sss" << std::endl;
	}
};
int main(){

#if 1
	std::string title = "Test";
	float width = 1280;
	float height = 720;
	TestApp app(title, { width, height });
	app.start();

	//__debugbreak();
	return 0;
#endif
}