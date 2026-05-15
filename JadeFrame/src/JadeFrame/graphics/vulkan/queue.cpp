#include "queue.h"
#include "physical_device.h"
#include "surface.h"

namespace JadeFrame {
namespace vulkan {

Queue::Queue(const LogicalDevice& device, u32 queue_family_index, u32 queue_index) {
    vkGetDeviceQueue(device.m_handle, queue_family_index, queue_index, &m_handle);
}

auto Queue::submit(const CommandBuffer& cmd_buffer) const -> void {
    assert(cmd_buffer.m_stage == CommandBuffer::STAGE::EXCECUTABLE);
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer.m_handle,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };
    VkResult result = vkQueueSubmit(m_handle, 1, &submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) { assert(false); }
    cmd_buffer.m_stage = CommandBuffer::STAGE::PENDING;

    // this->submit(cmd_buffer, nullptr, nullptr, nullptr);
}

auto Queue::submit(
    const CommandBuffer& cmd_buffer,
    const Semaphore*     wait_semaphore,
    const Semaphore*     signal_semaphore,
    const Fence*         fence
) const -> void {
    assert(cmd_buffer.m_stage == CommandBuffer::STAGE::EXCECUTABLE);

    const bool has_fenc = fence != nullptr;
    const bool has_wait_semaphore = wait_semaphore != nullptr;
    const bool has_signal_semaphore = signal_semaphore != nullptr;

    VkFence fence_handle = has_fenc ? fence->m_handle : nullptr;

    std::array<VkPipelineStageFlags, 1> wait_stages = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    const VkSubmitInfo info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = has_wait_semaphore ? 1_u32 : 0,
        .pWaitSemaphores = has_wait_semaphore ? &wait_semaphore->m_handle : nullptr,
        .pWaitDstStageMask = wait_stages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer.m_handle,
        .signalSemaphoreCount = has_signal_semaphore ? 1_u32 : 0,
        .pSignalSemaphores = has_signal_semaphore ? &signal_semaphore->m_handle : nullptr,
    };
    VkResult result = vkQueueSubmit(m_handle, 1, &info, fence_handle);
    if (result != VK_SUCCESS) { JF_ASSERT(false, to_string(result)); }
    cmd_buffer.m_stage = CommandBuffer::STAGE::PENDING;
}

auto Queue::wait_idle() const -> void {
    VkResult result = VK_SUCCESS;
    result = vkQueueWaitIdle(m_handle);
    if (result != VK_SUCCESS) { assert(false); }
}

auto Queue::present(VkPresentInfoKHR info) const -> VkResult {
    VkResult result = vkQueuePresentKHR(m_handle, &info);
    if (result != VK_SUCCESS) { assert(false); }
    return result;
}

auto Queue::present(
    const u32&       index,
    const Swapchain& swapchain,
    const Semaphore* semaphore
) const -> VkResult {

    const bool has_semaphore = semaphore != nullptr;

    const VkPresentInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = has_semaphore ? 1_u32 : 0_u32,
        .pWaitSemaphores = has_semaphore ? &semaphore->m_handle : VK_NULL_HANDLE,
        .swapchainCount = 1,
        .pSwapchains = &swapchain.m_handle,
        .pImageIndices = &index,
        .pResults = nullptr,
    };
    return vkQueuePresentKHR(m_handle, &info);
}

auto QueueFamily::supports_present(const Surface& surface) const -> bool {
    VkBool32 present_support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        m_physical_device->m_handle, m_index, surface.m_handle, &present_support
    );
    return (bool)present_support;
}

auto QueueFamily::query_queues(const LogicalDevice& device, u32 index) const -> Queue {
    return {device, m_index, index};
}

} // namespace vulkan
} // namespace JadeFrame