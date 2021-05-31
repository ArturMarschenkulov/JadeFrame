#pragma once
#include "JadeFrame/defines.h"
#include "vulkan_context.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "JadeFrame/graphics/shared.h"

class Vulkan_Renderer : public IRenderer {
public:
	auto set_context(Windows_Window* window)  -> void;
	auto main_loop() -> void;
	// Inherited via IRenderer
	virtual auto swap_buffer(HWND window_handle) const -> void override;
	virtual auto clear_background() const -> void override;
	virtual auto render_pushed(const Matrix4x4& view_projection) const -> void override;
private:
	//Vulkan_Context m_context;
	Vulkan_Context* m_context_p;


};