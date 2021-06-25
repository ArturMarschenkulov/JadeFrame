#include "vulkan_command_pool.h"
#include "vulkan_physical_device.h"

auto VulkanCommandPool::init(VkDevice device, const VulkanPhysicalDevice& physical_device) -> void {
	VkResult result;
	QueueFamilyIndices queue_family_indices = physical_device.m_queue_family_indices;

	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.pNext = nullptr;
	pool_info.flags = 0; // Optional
	pool_info.queueFamilyIndex = queue_family_indices.m_graphics_family.value();

	result = vkCreateCommandPool(device, &pool_info, nullptr, &m_command_pool);
	if (result != VK_SUCCESS) {
		__debugbreak();
		//throw std::runtime_error("failed to create command pool!");
	}

	m_device = device;
}

auto VulkanCommandPool::deinit() -> void {
	vkDestroyCommandPool(m_device, m_command_pool, nullptr);
}
