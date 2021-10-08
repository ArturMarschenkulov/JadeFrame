#include "pch.h"
#include "vulkan_queue.h"
#include "vulkan_sync_object.h"

namespace JadeFrame {
auto VulkanQueue::submit(const VkSubmitInfo& submit_info, const VulkanFence* p_fence) const -> void {
	VkResult result;
	result = vkQueueSubmit(m_handle, 1, &submit_info, p_fence->m_handle);
	if (result != VK_SUCCESS) __debugbreak();
}

auto VulkanQueue::wait_idle() const -> void {
	VkResult result;
	result = vkQueueWaitIdle(m_handle);
	if (result != VK_SUCCESS) __debugbreak();
}
auto VulkanQueue::present(VkPresentInfoKHR info, VkResult& result) const -> void {
	result = vkQueuePresentKHR(m_handle, &info);
	if (result != VK_SUCCESS) __debugbreak();
}
}