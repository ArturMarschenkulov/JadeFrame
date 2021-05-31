#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

struct HWND__;	typedef HWND__* HWND;
struct VulkanSurface {
private:
public:
	auto init(VkInstance instance, HWND window_handle) -> void;
public:
	VkSurfaceKHR m_surface;
};
