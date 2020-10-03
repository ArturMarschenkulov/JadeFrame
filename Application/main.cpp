#include<JadeFrame.h>
#include <variant>
#include <optional>


class App : public BaseApp {
public:
public:
	virtual auto setup() -> void override {}
	virtual auto update() -> void override {}
	virtual auto draw() -> void override {}
};

#include <iostream>
#include <thread>
int main(void) {
	App app;
	std::string title = "Test";
	float width = 1800;
	float height = 800;
	app.start(title, { width, height });

	return 0;
}

