#include "vulkan_surface.h"

#include <iostream>
#include <cassert>


auto VulkanSurface::init(VkInstance instance, HWND window_handle) -> void {
	VkResult result;

	VkWin32SurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.pNext = nullptr;
	create_info.flags = 0;
	create_info.hinstance = ::GetModuleHandleW(NULL);
	create_info.hwnd = window_handle;

	result = vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &m_surface);
	if (result != VK_SUCCESS) {
		std::cout << result << std::endl;
		__debugbreak();
		throw std::runtime_error("failed to create window surface!");
	}
}