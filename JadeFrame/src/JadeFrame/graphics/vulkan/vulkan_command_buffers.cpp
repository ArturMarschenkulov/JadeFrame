#include "pch.h"
#include "vulkan_logical_device.h"
#include "vulkan_command_buffers.h"
#include "vulkan_command_pool.h"
#include "vulkan_swapchain.h"

namespace JadeFrame {
auto VulkanCommandBuffers::init(
	const VulkanLogicalDevice& device,
	const VulkanCommandPool& command_pool,
	const size_t amount
) -> void {
	VkResult result;
	m_device = &device;
	m_command_pool = &command_pool;
	m_handles.resize(amount);

	const VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = command_pool.m_handle,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<u32>(m_handles.size()),
	};
	result = vkAllocateCommandBuffers(device.m_handle, &alloc_info, m_handles.data());
	if (result != VK_SUCCESS) __debugbreak();
}

auto VulkanCommandBuffers::deinit() -> void {
	vkFreeCommandBuffers(m_device->m_handle, m_command_pool->m_handle, static_cast<uint32_t>(m_handles.size()), m_handles.data());

}

auto VulkanCommandBuffers::begin_end_scope(size_t index, std::function<void()> func) -> void {
	VkResult result;
	const VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = {},
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = {},
	};

	result = vkBeginCommandBuffer(m_handles[index], &begin_info);
	if (result != VK_SUCCESS) __debugbreak();

	func();


	result = vkEndCommandBuffer(m_handles[index]);
	if (result != VK_SUCCESS) __debugbreak();



}

}
