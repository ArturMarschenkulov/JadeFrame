#pragma once
#include<JadeFrame.h>
namespace Test1 {
struct Example_99 : public BaseApp {
	Example_99(const std::string& title, const Vec2& size, const Vec2& position = { -1, -1 });
	virtual ~Example_99() = default;

	virtual auto on_init() -> void override;
	virtual auto on_update() -> void override;
	virtual auto on_draw() -> void override;

public:
	std::deque<Mesh> m_meshes;
	std::vector<Object> m_objs;

};
}

namespace Test1 {
Example_99::Example_99(const std::string& title, const Vec2& size, const Vec2& position)
	: BaseApp(title, size, position) {
	m_current_app_p = reinterpret_cast<Example_99*>(this);
}

auto Example_99::on_init() -> void {

	// Set Up Camera
	m_camera.othographic_mode(0, m_windows[0].m_size.x, m_windows[0].m_size.y, 0, -1, 1);

}
auto Example_99::on_update() -> void {

}
auto Example_99::on_draw() -> void {
}
}
using TestApp = Test1::Example_99;