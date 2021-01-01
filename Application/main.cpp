#include<JadeFrame.h>
#include<iostream>
#include <variant>
#include <map>

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



int main() {

	std::multimap<int, char> dict{
	{1, 'A'},
	{2, 'B'},
	{2, 'C'},
	{2, 'D'},
	{4, 'E'},
	{3, 'F'}
	};

	auto range = dict.equal_range(2);

	for (auto i = range.first; i != range.second; ++i) {
		std::cout << i->first << ": " << i->second << '\n';
	}
#if 1
	std::string title = "Test";
	float width = 1280;
	float height = 720;
	TestApp app(title, { width, height });
	app.start();
	return 0;
#endif
}