#pragma once
#include "JadeFrame/defines.h"
#include "vulkan_context.h"
#include "../shared.h"
namespace JadeFrame {

class Windows_Window;
class Object;
class Matrix4x4;
class Color;

class Vulkan_Renderer : public IRenderer {
public:
	Vulkan_Renderer(const Windows_Window& window);

	virtual auto set_clear_color(const Color& color) -> void override;

	virtual auto submit(const Object& obj) -> void override;
	virtual auto render(const Matrix4x4& view_projection) -> void override;
	virtual auto present()  -> void override;
	virtual auto clear_background() -> void override;
	virtual auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void override;
	virtual auto take_screenshot(const char* filename) -> void override;

	virtual auto main_loop() -> void override;

private:
	Vulkan_Context m_context;

private: //NOTE: probably temporary
	Matrix4x4 m_view_projection;


};
}