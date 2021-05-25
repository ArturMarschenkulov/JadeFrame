#pragma once
#include <vulkan/vulkan.h>
#include <vector>


struct HWND__;	typedef HWND__* HWND;
struct Vulkan_Context {
	Vulkan_Context() = default;
	Vulkan_Context(HWND window);
	~Vulkan_Context();

	VkInstance m_instance;
	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
	std::vector<VkExtensionProperties> m_extensions;
	std::vector<const char*> m_extension_names;
	std::vector<VkLayerProperties> m_layers;
	std::vector<VkPhysicalDevice> m_physical_devices;
	VkDevice m_device;
	VkQueue m_graphics_queue;
	VkSurfaceKHR m_surface;

private:
	auto query_extensions() -> std::vector<VkExtensionProperties>;
	auto query_physical_devices() -> std::vector<VkPhysicalDevice>;
	auto query_layers() -> std::vector<VkLayerProperties>;
};