#pragma
#include "../platform_shared.h"

namespace JadeFrame {


class Linux_Window : public IWindow {
public:
	virtual auto set_title(const std::string& title) -> void override {

	}
	virtual auto get_title() const -> std::string override {
		return {};
	}
};
}