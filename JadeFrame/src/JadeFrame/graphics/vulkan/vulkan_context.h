#pragma once
#include <vulkan/vulkan.h>

#include "vulkan_physical_device.h"
#include "vulkan_surface.h"
#include "vulkan_logical_device.h"

#include <vector>

class Windows_Window;

class VulkanInstance {

private:
	auto query_layers()->std::vector<VkLayerProperties>;
	auto query_extensions()->std::vector<VkExtensionProperties>;
	auto query_physical_devices() -> std::vector<VulkanPhysicalDevice>;
	auto setup_debug() -> void;
public:
	auto init(HWND window_handle) -> void;
	auto deinit() -> void;
public:
	VkInstance m_instance;
	HWND m_window_handle;

	std::vector<VkLayerProperties> m_layers;

	std::vector<VkExtensionProperties> m_extensions;
	std::vector<const char*> m_extension_names;

	VkDebugUtilsMessengerEXT m_debug_messenger;

	std::vector<VulkanPhysicalDevice> m_physical_devices;
	VulkanPhysicalDevice m_physical_device;
	VulkanLogicalDevice m_logical_device;

	VulkanSurface m_surface;
};

struct Vulkan_Context {
	//Vulkan_Context(const Vulkan_Context&) = delete;
	//Vulkan_Context(Vulkan_Context&&) = delete;
	//auto operator=(const Vulkan_Context&)->Vulkan_Context & = delete;
	//auto operator=(Vulkan_Context&&)->Vulkan_Context & = delete;

	Vulkan_Context() = default;
	Vulkan_Context(const Windows_Window& window);
	~Vulkan_Context();
public:
	VulkanInstance m_instance;
	HWND m_window_handle;
public:
	auto main_loop() -> void;
};