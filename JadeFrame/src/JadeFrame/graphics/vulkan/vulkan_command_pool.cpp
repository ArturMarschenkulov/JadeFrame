#include "pch.h"
#include "vulkan_command_pool.h"
#include "vulkan_physical_device.h"
#include "vulkan_logical_device.h"
#include "vulkan_command_buffers.h"

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

auto VulkanCommandPool::allocate_command_buffers(u32 amount) const -> std::vector<VulkanCommandBuffer> {
	VkResult result;

	std::vector<VkCommandBuffer> handles(amount);
	const VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = m_handle,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<u32>(handles.size()),
	};
	result = vkAllocateCommandBuffers(m_device->m_handle, &alloc_info, handles.data());
	if (result != VK_SUCCESS) __debugbreak();


	std::vector<VulkanCommandBuffer> command_buffers(handles.size());
	for(u32 i = 0; i < command_buffers.size(); i++) {
		command_buffers[i].m_handle = handles[i];
		command_buffers[i].m_device = m_device;
		command_buffers[i].m_command_pool = this;
	}
	return command_buffers;
}
auto VulkanCommandPool::allocate_command_buffer() const -> VulkanCommandBuffer {
	return this->allocate_command_buffers(1)[0];
}
auto VulkanCommandPool::free_command_buffers(const std::vector<VulkanCommandBuffer>& command_buffers) const -> void {
	for (u32 i = 0; i < command_buffers.size(); i++) {
		vkFreeCommandBuffers(m_device->m_handle, m_handle, 1, &command_buffers[i].m_handle);
	}
}
}