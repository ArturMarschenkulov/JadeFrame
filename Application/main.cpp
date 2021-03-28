#include<JadeFrame.h>
#include<string>

class App : public BaseApp {
public:
public:
	App(const std::string& title, const Vec2 pos)
		:BaseApp(title, pos) {
	}
	virtual auto on_init() -> void override {
		//m_renderer.gl_cache.set_clear_color({ 1.0, 0.0, 0.0 });
		//std::cout << "sss" << std::endl;
	}
	virtual auto on_update() -> void override {
		//std::cout << "sss" << std::endl;
	}
	virtual auto on_draw() -> void override {
		//std::cout << "sss" << std::endl;
	}
};
auto custom_simple_hash_0(const std::string& str) -> uint32_t {
	uint32_t hash = 0;
	for (auto& it : str) {
		hash = 37 * hash + 17 * static_cast<char>(it);
	}
	return hash;
}
constexpr auto custom_simple_hash_1(const char* str) -> uint32_t {
	uint32_t hash = 0;
	for (int i = 0; str[i] != '\0'; i++) {
		hash = 37 * hash + 17 * str[i];
	}
	return hash;
}
#include <intrin.h>

int main() {

#if 1
	using GApp = TestApp;
	JadeFrame jade_frame;

	std::string title = "Test";
	float width = 1280;
	float height = 720;
	GApp* app = new GApp(title, { width, height });

	jade_frame.add(app);
	jade_frame.run();
	return 0;
#endif
}