#include "vulkan_physical_device.h"

#include "vulkan_context.h"
#include "vulkan_surface.h"

#include <iostream>
#include <set>
#include <string>

namespace JadeFrame {

auto print_queue_families_info(VulkanPhysicalDevice physical_device) -> void {
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

			std::cout << "Queue familiy " << i << " has " << queue_families[i].queueCount << " queues capable of " << str << std::endl;
		}
	}
}
auto VulkanPhysicalDevice::init(VulkanSurface surface) -> void {
	VkResult result;

	vkGetPhysicalDeviceProperties(m_handle, &m_properties);
	vkGetPhysicalDeviceFeatures(m_handle, &m_features);
	vkGetPhysicalDeviceMemoryProperties(m_handle, &m_memory_properties);

	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_handle, surface.m_surface, &m_surface_capabilities);
	{ // Query Queue Family Properties
		u32 queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queue_family_count, nullptr);
		m_queue_family_properties.resize(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queue_family_count, m_queue_family_properties.data());
	}
	{ // Query_extension_properties
		u32 extension_count = 0;
		result = vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extension_count, nullptr);
		m_extension_properties.resize(extension_count);
		result = vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extension_count, m_extension_properties.data());
		if (VK_SUCCESS != result) __debugbreak();
	}
	{ // Query Surface Formats
		u32 format_count;
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_handle, surface.m_surface, &format_count, nullptr);
		if (VK_SUCCESS != result || (format_count == 0)) __debugbreak();

		m_surface_formats.resize(format_count);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_handle, surface.m_surface, &format_count, m_surface_formats.data());
		if (VK_SUCCESS != result) __debugbreak();
	}
	{ // Query Surface Present Modes
		u32 present_modes_count;
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_handle, surface.m_surface, &present_modes_count, nullptr);
		if (VK_SUCCESS != result || (present_modes_count == 0)) __debugbreak();

		m_present_modes.resize(present_modes_count);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_handle, surface.m_surface, &present_modes_count, m_present_modes.data());
		if (VK_SUCCESS != result) __debugbreak();

	}
	m_extension_support = this->check_extension_support(g_device_extensions);
	m_queue_family_indices = this->find_queue_families(surface);
	print_queue_families_info(*this);
}

auto VulkanPhysicalDevice::check_extension_support(const std::vector<const char*>& extensions) -> bool {
	std::set<std::string> required_extensions(extensions.begin(), extensions.end());
	for (u32 i = 0; i < m_extension_properties.size(); i++) {
		required_extensions.erase(m_extension_properties[i].extensionName);
	}
	return required_extensions.empty();
}

auto VulkanPhysicalDevice::find_queue_families(VulkanSurface surface) -> QueueFamilyIndices {
	VkResult result;

	QueueFamilyIndices indices;
	for (u32 i = 0; i < m_queue_family_properties.size(); i++) {
		if (m_queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.m_graphics_family = i;
		}
		VkBool32 present_support = false;

		result = vkGetPhysicalDeviceSurfaceSupportKHR(m_handle, i, surface.m_surface, &present_support);
		if (result != VK_SUCCESS) __debugbreak();


		if (present_support) {
			indices.m_present_family = i;
		}
		if (indices.is_complete()) {
			break;
		}
	}
	return indices;
}
}