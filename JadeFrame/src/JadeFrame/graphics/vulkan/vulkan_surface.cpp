#include "pch.h"
#include "vulkan_surface.h"

#include <cassert>

namespace JadeFrame {
auto VulkanSurface::init(VkInstance instance, HWND window_handle) -> void {
	VkResult result;

	const VkWin32SurfaceCreateInfoKHR create_info = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.hinstance = ::GetModuleHandleW(NULL),
		.hwnd = window_handle,
	};

	result = vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) {
		__debugbreak();
		throw std::runtime_error("failed to create window surface!");
	}
}
}