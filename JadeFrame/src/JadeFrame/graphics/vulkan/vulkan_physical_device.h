#pragma once
#include <vulkan/vulkan.h>


#include <JadeFrame/defines.h>

#include <optional>
#include <vector>

namespace JadeFrame {

class VulkanSurface;



struct QueueFamilyIndices {
	std::optional<u32> m_graphics_family;
	std::optional<u32> m_present_family;
	auto is_complete() -> bool {
		return m_graphics_family.has_value() && m_present_family.has_value();
	}

};
struct SurfaceSupportDetails {
	VkSurfaceCapabilitiesKHR        m_capabilities;
	std::vector<VkSurfaceFormatKHR> m_formats;
	std::vector<VkPresentModeKHR>   m_present_modes;
};
class VulkanInstance;
class VulkanPhysicalDevice {
private:
public:
	auto init(VulkanInstance& instance, const VulkanSurface& surface) -> void;
	auto check_extension_support(const std::vector<const char*>& extensions) -> bool;
	auto find_queue_families(VulkanSurface surface)->QueueFamilyIndices;
	//auto choose_swap_surface_format() const -> VkSurfaceFormatKHR;
	//auto choose_swap_present_mode() const -> VkPresentModeKHR;
	//auto choose_swap_extent() const -> VkExtent2D;
	auto find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties) const -> u32;

public:
	VulkanInstance* m_instance_p = nullptr;
	VkPhysicalDevice m_handle;
	VkPhysicalDeviceProperties m_properties = {};
	VkPhysicalDeviceFeatures m_features = {};
	VkPhysicalDeviceMemoryProperties m_memory_properties = {};
	std::vector<VkQueueFamilyProperties> m_queue_family_properties;
	std::vector<VkExtensionProperties> m_extension_properties;
	std::vector<const char*> m_device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	bool m_extension_support;
	QueueFamilyIndices m_queue_family_indices;
	SurfaceSupportDetails m_surface_support_details;
	//VkSurfaceCapabilitiesKHR m_surface_capabilities;
	//std::vector<VkSurfaceFormatKHR> m_surface_formats;
	//std::vector<VkPresentModeKHR> m_present_modes;
};
}