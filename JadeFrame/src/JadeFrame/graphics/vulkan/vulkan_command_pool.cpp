#include "pch.h"
#include "vulkan_command_pool.h"
#include "vulkan_physical_device.h"
#include "vulkan_logical_device.h"

namespace JadeFrame {

auto VulkanCommandPool::init(const VulkanLogicalDevice& device, const QueueFamilyIndices& queue_family_indices) -> void {
	m_device = &device;
	VkResult result;
	//QueueFamilyIndices queue_family_indices = physical_device.m_queue_family_indices;

	const VkCommandPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0, // Optional
		.queueFamilyIndex = queue_family_indices.m_graphics_family.value(),
	};

	result = vkCreateCommandPool(device.m_handle, &pool_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();

}

auto VulkanCommandPool::deinit() -> void {
	vkDestroyCommandPool(m_device->m_handle, m_handle, nullptr);
}
auto VulkanCommandPool::allocate_command_buffers(u32 amount) -> VulkanCommandBuffers {
	VulkanCommandBuffers command_buffers;
	command_buffers.init(*m_device, *this, amount);
	return command_buffers;
	//VkResult result;
	//m_device = &device;
	//m_command_pool = &command_pool;
	//m_handles.resize(amount);

	//const VkCommandBufferAllocateInfo alloc_info = {
	//	.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	//	.pNext = nullptr,
	//	.commandPool = command_pool.m_handle,
	//	.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	//	.commandBufferCount = static_cast<u32>(m_handles.size()),
	//};
	//result = vkAllocateCommandBuffers(device.m_handle, &alloc_info, m_handles.data());
	//if (result != VK_SUCCESS) __debugbreak();

	//return VulkanCommandBuffer();
}
}