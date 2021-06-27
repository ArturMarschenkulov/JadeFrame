#pragma once
#include "JadeFrame/defines.h"
#include "vulkan_context.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "JadeFrame/graphics/shared.h"
namespace JadeFrame {
class Vulkan_Renderer : public IRenderer {
public:
	auto set_context(const Windows_Window& window)  -> void;
	auto main_loop() -> void;

	virtual auto swap_buffer(HWND window_handle) const -> void override;
	virtual auto clear_background() const -> void override;
	virtual auto render(const Matrix4x4& view_projection) const -> void override;
	virtual auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void override;
private:
	//Vulkan_Context m_context;
	Vulkan_Context* m_context_p;


};
}