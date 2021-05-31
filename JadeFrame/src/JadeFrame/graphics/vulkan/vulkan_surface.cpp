#include "vulkan_surface.h"

#include <iostream>


auto VulkanSurface::init(VkInstance instance, HWND window_handle) -> void {
	//PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR2;
	//vkCreateWin32SurfaceKHR2 = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkCreateWin32SurfaceKHR");
	//if (!vkCreateWin32SurfaceKHR2) {
	//	std::cout << "lll" << std::endl;
	//	__debugbreak();
	//}

	VkWin32SurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	//create_info.pNext;
	//create_info.flags;
	create_info.hinstance = ::GetModuleHandleW(NULL);
	create_info.hwnd = window_handle;

	VkResult result = vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &m_surface);
	if (result != VK_SUCCESS) {
		std::cout << result << std::endl;
		__debugbreak();
		throw std::runtime_error("failed to create window surface!");
	}
}