#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_surface.h"

#include <JadeFrame/defines.h>

#include <optional>
#include <vector>

const std::vector<const char*> g_device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

struct QueueFamilyIndices {
	std::optional<u32> graphics_family;
	std::optional<u32> present_family;
	auto is_complete() -> bool {
		return graphics_family.has_value() && present_family.has_value();
	}

};
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR m_capabilities;
	std::vector<VkSurfaceFormatKHR> m_formats;
	std::vector<VkPresentModeKHR> m_present_modes;
};
struct VulkanPhysicalDevice {
private:
public:
	auto init(VulkanSurface surface) -> void;
	auto check_extension_support(const std::vector<const char*> extensions) -> bool;
	auto find_queue_families(VulkanSurface surface) -> QueueFamilyIndices;
public:
	VkPhysicalDevice m_handle;
	VkPhysicalDeviceProperties m_properties = {};
	VkPhysicalDeviceFeatures m_features = {};
	VkPhysicalDeviceMemoryProperties m_memory_properties = {};
	//VkSurfaceCapabilitiesKHR m_surfaceCapabilities = {};
	std::vector<VkQueueFamilyProperties> m_queue_family_properties;
	//std::vector<VkSurfaceFormatKHR> m_surface_formats;
	//std::vector<VkPresentModeKHR> m_present_modes;
	std::vector<VkExtensionProperties> m_extension_properties;

	bool m_extension_support;
	QueueFamilyIndices m_queue_family_indices;
	//SwapChainSupportDetails m_swapchain_support_details;
	VkSurfaceCapabilitiesKHR m_surface_capabilities;
	std::vector<VkSurfaceFormatKHR> m_surface_formats;
	std::vector<VkPresentModeKHR> m_present_modes;
};