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
auto VulkanPhysicalDevice::init(VulkanInstance& instance, const VulkanSurface& surface) -> void {
	m_instance_p = &instance;
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
	m_extension_support = this->check_extension_support(m_device_extensions);
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
auto VulkanPhysicalDevice::choose_swap_surface_format() const -> VkSurfaceFormatKHR {
	
	for (u32 i = 0; i < m_surface_formats.size(); i++) {
		if (m_surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			m_surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return m_surface_formats[i];
		}
	}
	return m_surface_formats[0];
}
auto VulkanPhysicalDevice::choose_swap_present_mode() const -> VkPresentModeKHR {
	for (u32 i = 0; i < m_present_modes.size(); i++) {
		if (m_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return m_present_modes[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}
auto VulkanPhysicalDevice::choose_swap_extent() const -> VkExtent2D {
	//vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_handle, surface.m_surface, &m_surface_capabilities);
	if (false/*m_surface_capabilities.currentExtent.width != UINT32_MAX*/) {
		return m_surface_capabilities.currentExtent;
	} else {
		RECT area;
		GetClientRect(m_instance_p->m_window_handle, &area);
		i32 width = area.right;
		i32 height = area.bottom;
		//glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actual_extent = {
			static_cast<u32>(width),
			static_cast<u32>(height)
		};

		//actual_extent.width = std::max(m_surface_capabilities.minImageExtent.width, std::min(m_surface_capabilities.maxImageExtent.width, actual_extent.width));
		//actual_extent.height = std::max(m_surface_capabilities.minImageExtent.height, std::min(m_surface_capabilities.maxImageExtent.height, actual_extent.height));


		//actual_extent.width = std::clamp(actual_extent.width, m_surface_capabilities.minImageExtent.width, m_surface_capabilities.maxImageExtent.width);
		//actual_extent.height = std::clamp(actual_extent.height, m_surface_capabilities.minImageExtent.height, m_surface_capabilities.maxImageExtent.height);

		return actual_extent;
	}
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
}