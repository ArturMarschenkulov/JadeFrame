#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {
class VulkanFence;

class VulkanQueue {
public:
	auto submit(const VkSubmitInfo& submit_info, const VulkanFence* p_fence) const -> void;
	auto submit(const VkCommandBuffer& cmd_buffer, const std::array<VkSemaphore, 1>& wait_semaphores, const std::array<VkSemaphore, 1>& signal_semaphore) -> void;
	auto wait_idle() const -> void;
	auto present(VkPresentInfoKHR info, VkResult& result) const -> void;
public:
	VkQueue m_handle = VK_NULL_HANDLE;
};


inline auto VulkanQueue::submit(const VkCommandBuffer& cmd_buffer, const std::array<VkSemaphore, 1>& wait_semaphores, const std::array<VkSemaphore, 1>& signal_semaphores) -> void {
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	const VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = (uint32_t)wait_semaphores.size(),
		.pWaitSemaphores = wait_semaphores.data(),
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd_buffer,
		.signalSemaphoreCount = (uint32_t)signal_semaphores.size(),
		.pSignalSemaphores = signal_semaphores.data(),
	};
	/*this->submit(submit_info);*/
	VkResult result;
	result = vkQueueSubmit(m_handle, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) __debugbreak();
}
}