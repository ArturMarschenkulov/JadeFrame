#pragma once
#include <vulkan/vulkan.h>

#include "vulkan_physical_device.h"
#include "vulkan_surface.h"
#include "vulkan_logical_device.h"

#include <vector>


namespace JadeFrame {
class VulkanInstance {

private:
	auto query_layers()->std::vector<VkLayerProperties>;
	auto query_extensions()->std::vector<VkExtensionProperties>;
	auto query_physical_devices()->std::vector<VulkanPhysicalDevice>;
	auto setup_debug() -> void;
	auto check_validation_layer_support(const std::vector<VkLayerProperties>& available_layers) -> bool;
public:
	VulkanInstance() = default;
	auto init(HWND window_handle) -> void;
	auto deinit() -> void;
public:
	VkInstance m_instance;
	HWND m_window_handle;

	std::vector<VkLayerProperties> m_layers;

	std::vector<VkExtensionProperties> m_extensions;
	std::vector<const char*> m_extension_names;

	VkDebugUtilsMessengerEXT m_debug_messenger;
#ifdef NDEBUG
	const bool enable_validation_layers = false;
#else
	const bool m_enable_validation_layers = true;
#endif
	const std::vector<const char*> m_validation_layers;

	std::vector<VulkanPhysicalDevice> m_physical_devices;
	VulkanPhysicalDevice m_physical_device;
	VulkanLogicalDevice m_logical_device;

	VulkanSurface m_surface;
};
}