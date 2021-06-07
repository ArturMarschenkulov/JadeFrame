
#include "vulkan_renderer.h"

auto Vulkan_Renderer::set_context(const Windows_Window& window) -> void {
	m_context_p = new Vulkan_Context(window); //TODO: Consider how to handle this in a different way as this seems too unrobust
}

auto Vulkan_Renderer::main_loop() -> void {
	m_context_p->main_loop();
}

auto Vulkan_Renderer::swap_buffer(HWND window_handle) const -> void {

}

auto Vulkan_Renderer::clear_background() const -> void {
}

auto Vulkan_Renderer::render(const Matrix4x4& view_projection) const -> void {
}

auto Vulkan_Renderer::set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void {

}
