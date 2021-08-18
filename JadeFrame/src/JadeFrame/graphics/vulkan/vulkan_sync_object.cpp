#include "pch.h"
#include "vulkan_sync_object.h"
namespace JadeFrame {
auto VulkanFence::init(VkDevice device) -> void {
	m_device = device;
	VkResult result;

	const VkFenceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	result = vkCreateFence(device, &create_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();
}

auto VulkanFence::deinit() -> void {

	vkDestroyFence(m_device, m_handle, nullptr);
}

auto VulkanSemaphore::init(VkDevice device) -> void {
	m_device = device;
	VkResult result;

	const VkSemaphoreCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
	};

	result = vkCreateSemaphore(device, &create_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();
}

auto VulkanSemaphore::deinit() -> void {

	vkDestroySemaphore(m_device, m_handle, nullptr);
}

}
