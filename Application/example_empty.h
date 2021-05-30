#pragma once
#include<JadeFrame.h>

struct Example_Empty : public BaseApp {
	Example_Empty(const std::string& title, const Vec2& size, const Vec2& position = { -1, -1 });
	virtual ~Example_Empty() = default;

	virtual auto on_init() -> void override;
	virtual auto on_update() -> void override;
	virtual auto on_draw() -> void override;

public:
	std::deque<Mesh> m_meshes;
	std::vector<Object> m_objs;
};


Example_Empty::Example_Empty(const std::string& title, const Vec2& size, const Vec2& position)
	: BaseApp(title, size, position) {
	m_current_app_p = reinterpret_cast<Example_Empty*>(this);
}
//m_camera.othographic_mode(0, m_windows[0].m_size.x, m_windows[0].m_size.y, 0, -1, 1);
auto Example_Empty::on_init() -> void {

}
auto Example_Empty::on_update() -> void {

}
auto Example_Empty::on_draw() -> void {

}
using TestApp = Example_Empty;