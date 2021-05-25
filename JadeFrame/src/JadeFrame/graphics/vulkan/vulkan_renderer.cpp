
#include "vulkan_renderer.h"

auto Vulkan_Renderer::set_context(const HWND& window) -> void {
	m_context = Vulkan_Context(window);
}