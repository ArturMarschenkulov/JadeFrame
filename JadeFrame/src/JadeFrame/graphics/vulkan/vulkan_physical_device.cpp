#include "pch.h"
#include "vulkan_physical_device.h"

#include "vulkan_context.h"
#include "vulkan_instance.h"
#include "vulkan_surface.h"

#include <set>
#include <string>
#include <Windows.h> // TODO: Try to remove it. Used in "choose_swap_extent()"
#include "JadeFrame/utils/logger.h"

#undef min
#undef max

namespace JadeFrame {

static auto print_queue_families_info(VulkanPhysicalDevice physical_device) -> void {
	auto& queue_families = physical_device.m_queue_family_properties;
	{
		for (u32 i = 0; i < queue_families.size(); i++) {
			std::string str = "{ ";
			if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				str += "Graphics ";
			}
			if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
				str += "Compute ";
			}
			if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
				str += "Transfer ";
			}
			if (queue_families[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
				str += "SparseBinding ";
			}
			if (queue_families[i].queueFlags & VK_QUEUE_PROTECTED_BIT) {
				str += "Protected ";
			}
			str += "}";
			Logger::log("Queue family {} has {} queues capable of {}", 
				i, queue_families[i].queueCount, str
			);
		}
	}
}

static auto query_surface_support_details(const VulkanPhysicalDevice& physical_device, const VulkanSurface& surface) ->SurfaceSupportDetails {
	VkResult result;
	SurfaceSupportDetails surface_support_details;
	u32 count = 0;

	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device.m_handle, surface.m_handle, &surface_support_details.m_capabilities);
	


	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device.m_handle, surface.m_handle, &count, nullptr);
	if (VK_SUCCESS != result || (count == 0)) __debugbreak();

	surface_support_details.m_formats.resize(count);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device.m_handle, surface.m_handle, &count, surface_support_details.m_formats.data());
	if (VK_SUCCESS != result) __debugbreak();



	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device.m_handle, surface.m_handle, &count, nullptr);
	if (VK_SUCCESS != result || (count == 0)) __debugbreak();

	surface_support_details.m_present_modes.resize(count);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device.m_handle, surface.m_handle, &count, surface_support_details.m_present_modes.data());
	if (VK_SUCCESS != result) __debugbreak();

	return surface_support_details;
}
//template<typename Flag>
//auto decode_bitmask(const Flag&) -> std::vector<Flag> {
//
//}
auto VulkanPhysicalDevice::init(VulkanInstance& instance, const VulkanSurface& surface) -> void {
	m_instance_p = &instance;
	VkResult result;
	vkGetPhysicalDeviceProperties(m_handle, &m_properties);
	vkGetPhysicalDeviceFeatures(m_handle, &m_features);
	vkGetPhysicalDeviceMemoryProperties(m_handle, &m_memory_properties);

	m_surface_support_details = query_surface_support_details(*this, surface);

	m_queue_families = this->query_queue_families(surface);
	m_queue_family_indices = this->find_queue_families(m_queue_families);
	print_queue_families_info(*this);

	{ // Query_extension_properties
		u32 count = 0;
		result = vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, nullptr);
		m_extension_properties.resize(count);
		result = vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, m_extension_properties.data());
		if (VK_SUCCESS != result) __debugbreak();
	}
	m_extension_support = this->check_extension_support(m_device_extensions);

}

auto VulkanPhysicalDevice::check_extension_support(const std::vector<const char*>& extensions) -> bool {
	std::set<std::string> required_extensions(extensions.begin(), extensions.end());
	for (u32 i = 0; i < m_extension_properties.size(); i++) {
		required_extensions.erase(m_extension_properties[i].extensionName);
	}
	return required_extensions.empty();
}


auto VulkanPhysicalDevice::find_queue_families(const std::vector<VulkanQueueFamily>& queue_families) -> QueueFamilyIndices {
	VkResult result;

	QueueFamilyIndices indices;
	for (u32 i = 0; i < queue_families.size(); i++) {
		if (queue_families[i].m_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.m_graphics_family = queue_families[i].m_index;
		}
		VkBool32 present_support = false;
		present_support = queue_families[i].m_present_support;


		if (present_support) {
			indices.m_present_family = queue_families[i].m_index;
		}
		if (indices.is_complete()) {
			break;
		}
	}
	return indices;
}
auto VulkanPhysicalDevice::find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties) const -> u32 {
	const VkPhysicalDeviceMemoryProperties& mem_props = m_memory_properties;
	for (u32 i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");
}
auto VulkanPhysicalDevice::query_queue_families(const VulkanSurface& surface) -> std::vector<VulkanQueueFamily> {
	u32 count = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, nullptr);
	
	std::vector<VkQueueFamilyProperties> queue_family_properties;
	queue_family_properties.resize(count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, queue_family_properties.data());

	std::vector<VulkanQueueFamily> families;
	families.resize(count);
	for(u32 i = 0; i < count; i++) {
		families[i].m_index = i;
		families[i].m_properties = queue_family_properties[i];
		vkGetPhysicalDeviceSurfaceSupportKHR(m_handle, families[i].m_index, surface.m_handle, &families[i].m_present_support);
	}

	return families;
}
}