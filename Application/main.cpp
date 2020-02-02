#include<JadeFrame.h>


class App : public BaseApp {
public:
public:
	void setup() {}
	void update() {}
	void draw() {}
};


int main(void) {

	BaseApp* app = new App();
	app->initApp();
	app->runApp();
	return 0;
}