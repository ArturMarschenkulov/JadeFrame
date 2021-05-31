#include "vulkan_physical_device.h"
#include <set>
#include <string>
#include "vulkan_context.h"
#include <iostream>


auto get_queue_families_info(VulkanPhysicalDevice physical_device) -> void {
	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device.m_handle, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families;
	queue_families.resize(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device.m_handle, &queue_family_count, queue_families.data());

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
auto VulkanPhysicalDevice::init(VulkanSurface surface) -> void {
	vkGetPhysicalDeviceProperties(m_handle, &m_properties);
	vkGetPhysicalDeviceFeatures(m_handle, &m_features);
	vkGetPhysicalDeviceMemoryProperties(m_handle, &m_memory_properties);
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_handle, surface.m_surface, &m_swapchain_support_details.m_capabilities);
	{
		u32 queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queue_family_count, nullptr);
		m_queue_family_properties.resize(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queue_family_count, m_queue_family_properties.data());
	}
	{
		uint32_t extension_count = 0;
		vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extension_count, nullptr);
		m_extension_properties.resize(extension_count);
		if (VK_SUCCESS != vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extension_count, m_extension_properties.data())) {
			__debugbreak();
		}
	}
	{
		u32 queue_family_properties_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queue_family_properties_count, nullptr);
		m_queue_family_properties.resize(queue_family_properties_count);
		vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queue_family_properties_count, m_queue_family_properties.data());
	}
	{
		uint32_t format_count;
		if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(m_handle, surface.m_surface, &format_count, nullptr)) {
			__debugbreak();
		}
		if (format_count != 0) {
			m_swapchain_support_details.m_formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(m_handle, surface.m_surface, &format_count, m_swapchain_support_details.m_formats.data());
		}
	}
	{
		uint32_t present_mode_count;
		if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(m_handle, surface.m_surface, &present_mode_count, nullptr)) {
			__debugbreak();
		}
		if (present_mode_count != 0) {
			m_swapchain_support_details.m_present_modes.resize(present_mode_count);
			if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(m_handle, surface.m_surface, &present_mode_count, m_swapchain_support_details.m_present_modes.data())) {
				__debugbreak();

			}
		}
	}
	//m_extension_properties = this->query_extension_properties();
	m_extension_support = this->check_extension_support();
	m_queue_family_indices = this->find_queue_families(surface);
	//m_swapchain_support_details = this->query_swapchain_support(surface);
	get_queue_families_info(*this);
}

auto VulkanPhysicalDevice::query_extension_properties() -> std::vector< VkExtensionProperties> {
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> available_extensions(extension_count);
	if (VK_SUCCESS != vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &extension_count, available_extensions.data())) {
		__debugbreak();
	}
	return available_extensions;
}
auto VulkanPhysicalDevice::check_extension_support() -> bool {
	std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
	for (u32 i = 0; i < m_extension_properties.size(); i++) {
		required_extensions.erase(m_extension_properties[i].extensionName);
	}
	return required_extensions.empty();
}

auto VulkanPhysicalDevice::query_swapchain_support(VulkanSurface surface) -> SwapChainSupportDetails {
	SwapChainSupportDetails details;
	auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_handle, surface.m_surface, &details.m_capabilities);
	if (VK_SUCCESS != res) {
		__debugbreak();
	}

	uint32_t format_count;
	if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(m_handle, surface.m_surface, &format_count, nullptr)) {
		__debugbreak();
	}
	if (format_count != 0) {
		details.m_formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_handle, surface.m_surface, &format_count, details.m_formats.data());
	}

	uint32_t present_mode_count;
	if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(m_handle, surface.m_surface, &present_mode_count, nullptr)) {
		__debugbreak();
	}
	if (present_mode_count != 0) {
		details.m_present_modes.resize(present_mode_count);
		if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(m_handle, surface.m_surface, &present_mode_count, details.m_present_modes.data())) {
			__debugbreak();

		}
	}
	return details;
}
auto VulkanPhysicalDevice::find_queue_families(VulkanSurface surface) -> QueueFamilyIndices {
	QueueFamilyIndices indices;

	u32 queue_family_properties_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queue_family_properties_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families;
	queue_families.resize(queue_family_properties_count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queue_family_properties_count, queue_families.data());

	for (u32 i = 0; i < queue_families.size(); i++) {
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}
		VkBool32 present_support = false;
		if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceSupportKHR(m_handle, i, surface.m_surface, &present_support)) {
			__debugbreak();
		}

		if (present_support) {
			indices.present_family = i;
		}
		if (indices.is_complete()) {
			break;
		}
	}
	return indices;
}