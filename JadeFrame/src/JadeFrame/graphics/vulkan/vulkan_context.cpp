
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan_context.h"
#include "vulkan_shared.h"
#include "vulkan_physical_device.h"

#include "JadeFrame/platform/windows/windows_window.h"


#include "JadeFrame/defines.h"

#include <iostream>

#include <set>
#include <JadeFrame/base_app.h>

namespace JadeFrame {


//static auto get_required_instance_extensions(u32* count) -> const char* {
//    const char* extensions[2];
//    extensions[0] = "VK_KHR_surface";
//    extensions[1] = "VK_KHR_win32_surface";
//    return *extensions;
//}

struct VulkanVersion {
	u32 variant;
	u32 major;
	u32 minor;
	u32 patch;
};
static auto vulkan_get_api_version(u32 version) -> VulkanVersion {
	VulkanVersion result;
	result.variant = VK_API_VERSION_VARIANT(version);
	result.major = VK_API_VERSION_MAJOR(version);
	result.minor = VK_API_VERSION_MINOR(version);
	result.patch = VK_API_VERSION_PATCH(version);
	return result;
}
static auto vulkan_get_device_type_string(const VkPhysicalDeviceType& device_type) -> const char* {
	const char* result = "";
	switch (device_type) {
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			result = "VK_PHYSICAL_DEVICE_TYPE_OTHER"; break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_CPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
			__debugbreak();
			result = ""; break;
	}
	return result;
}


Vulkan_Context::Vulkan_Context(const Windows_Window& window) {
	if (window.m_is_graphics_api_init == true) {
		if (window.m_graphics_api != Windows_Window::GRAPHICS_API::VULKAN) {
			window.recreate();
		}
	}
	m_window_handle = window.m_window_handle;
	m_instance.init(window.m_window_handle);
	window.m_graphics_api = Windows_Window::GRAPHICS_API::VULKAN;
	this->main_loop();
}

Vulkan_Context::~Vulkan_Context() {
	m_instance.m_logical_device.deinit();
	m_instance.deinit();
}


auto Vulkan_Context::main_loop() -> void {
	VkResult result;

	const std::vector<VVertex> vertices = {
		{{-0.5f, -0.5f}, {+1.0f, +0.0f, +0.0f}},
		{{+0.5f, -0.5f}, {+0.0f, +1.0f, +0.0f}},
		{{+0.5f, +0.5f}, {+0.0f, +0.0f, +1.0f}},

		{{-0.5f, +0.5f}, {+1.0f, +1.0f, +1.0f}},
	};
	const std::vector<u16> indices = {
		0, 1, 2,
		2, 3, 0,
	};

	while (true) {
		JadeFrameInstance::get_singleton()->m_apps[0]->poll_events();
		m_instance.m_logical_device.draw_frame();
	}
	result = vkDeviceWaitIdle(m_instance.m_logical_device.m_handle);
}







}