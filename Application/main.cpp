#include<JadeFrame.h>


class App : public BaseApp {
public:
public:
	void setup() {}
	void update() {}
	void draw() {}
};


int main(void) {
	std::string t = "Test";
	BaseApp* app = new App();
	app->initApp(t, 600, 600);
	app->runApp();
	return 0;
}