#include "vulkan_command_pool.h"
#include "vulkan_physical_device.h"

namespace JadeFrame {

auto VulkanCommandPool::init(VkDevice device, const VulkanPhysicalDevice& physical_device) -> void {
	VkResult result;
	QueueFamilyIndices queue_family_indices = physical_device.m_queue_family_indices;

	const VkCommandPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0, // Optional
		.queueFamilyIndex = queue_family_indices.m_graphics_family.value(),
	};

	result = vkCreateCommandPool(device, &pool_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();

	m_device = device;
}

auto VulkanCommandPool::deinit() -> void {
	vkDestroyCommandPool(m_device, m_handle, nullptr);
}
}
