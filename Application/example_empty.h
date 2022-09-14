#pragma once
#include<JadeFrame.h>
namespace JadeFrame {
struct Example_Empty : public BaseApp {
	Example_Empty(const DESC& desc);
	virtual ~Example_Empty() = default;

	virtual auto on_init() -> void override;
	virtual auto on_update() -> void override;
	virtual auto on_draw() -> void override;

public:
	// std::deque<Mesh> m_meshes;
	// std::vector<Object> m_objs;
};


Example_Empty::Example_Empty(const DESC& desc)
	: BaseApp(desc) {
	//m_current_app_p = reinterpret_cast<Example_Empty*>(this);
}
auto Example_Empty::on_init() -> void {

}
auto Example_Empty::on_update() -> void {

}
auto Example_Empty::on_draw() -> void {

}
using TestApp = Example_Empty;
}