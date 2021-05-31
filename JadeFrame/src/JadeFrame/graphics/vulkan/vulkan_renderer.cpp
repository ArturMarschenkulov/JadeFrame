
#include "vulkan_renderer.h"

auto Vulkan_Renderer::set_context(Windows_Window* window) -> void {
	m_context_p = new Vulkan_Context(window); //TODO: Consider how to handle this in a different way as this seems too unrobust
}

auto Vulkan_Renderer::main_loop() -> void {
	m_context_p->main_loop();
}

auto Vulkan_Renderer::swap_buffer(HWND window_handle) const -> void {
}

auto Vulkan_Renderer::clear_background() const -> void {
}

auto Vulkan_Renderer::render_pushed(const Matrix4x4& view_projection) const -> void {
}
