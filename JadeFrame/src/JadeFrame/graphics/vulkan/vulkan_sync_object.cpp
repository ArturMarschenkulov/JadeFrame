#include "pch.h"
#include "vulkan_sync_object.h"
#include "vulkan_logical_device.h"
#include "JadeFrame/utils/assert.h"
namespace JadeFrame {
auto VulkanFence::init(const VulkanLogicalDevice& device) -> void {
	m_device = &device;
	VkResult result;

	const VkFenceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	result = vkCreateFence(device.m_handle, &create_info, nullptr, &m_handle);
	JF_ASSERT(result == VK_SUCCESS, "");
}

auto VulkanFence::deinit() -> void {

	vkDestroyFence(m_device->m_handle, m_handle, nullptr);
}

auto VulkanFence::wait_for_fences() -> void {
	VkResult result;
	result = vkWaitForFences(m_device->m_handle, 1, &m_handle, VK_TRUE, UINT64_MAX);
	JF_ASSERT(result == VK_SUCCESS, "");

}

auto VulkanFence::reset() -> void {
	VkResult result;
	result = vkResetFences(m_device->m_handle, 1, &m_handle);
	JF_ASSERT(result == VK_SUCCESS, "");
}

auto VulkanSemaphore::init(const VulkanLogicalDevice& device) -> void {
	m_device = &device;
	VkResult result;

	const VkSemaphoreCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
	};

	result = vkCreateSemaphore(device.m_handle, &create_info, nullptr, &m_handle);
	JF_ASSERT(result == VK_SUCCESS, "");
}

auto VulkanSemaphore::deinit() -> void {

	vkDestroySemaphore(m_device->m_handle, m_handle, nullptr);
}

}
