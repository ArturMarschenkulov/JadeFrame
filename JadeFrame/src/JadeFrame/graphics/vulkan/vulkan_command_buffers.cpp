#include "pch.h"
#include "vulkan_logical_device.h"
#include "vulkan_command_buffers.h"
#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_physical_device.h"
#include "JadeFrame/utils/assert.h"

namespace JadeFrame {

auto VulkanCommandBuffer::record_begin() -> void {
    VkResult                       result;
    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    result = vkBeginCommandBuffer(m_handle, &begin_info);
    JF_ASSERT(result == VK_SUCCESS, "");
}

auto VulkanCommandBuffer::record_end() -> void {
    VkResult result;
    result = vkEndCommandBuffer(m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
}

auto VulkanCommandBuffer::render_pass_begin(
    const VulkanFramebuffer& framebuffer, const VulkanRenderPass& render_pass, const VkExtent2D& extent,
    VkClearValue clear_color) -> void {
    const VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass.m_handle,
        .framebuffer = framebuffer.m_handle,
        .renderArea =
            {
                         .offset = {0, 0},
                         .extent = extent,
                         },
        .clearValueCount = 1,
        .pClearValues = &clear_color,
    };

    vkCmdBeginRenderPass(m_handle, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

auto VulkanCommandBuffer::render_pass_end() -> void { vkCmdEndRenderPass(m_handle); }


auto VulkanCommandBuffer::reset() -> void {
    VkResult                  result;
    VkCommandBufferResetFlags flags = {};

    result = vkResetCommandBuffer(m_handle, flags);
    JF_ASSERT(result == VK_SUCCESS, "");
}



auto VulkanCommandPool::init(const VulkanLogicalDevice& device, const QueueFamilyIndex& queue_family_index) -> void {
    m_device = &device;
    VkResult result;

    const VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // Optional
        .queueFamilyIndex = queue_family_index /*queue_family_indices.m_graphics_family.value()*/,
    };

    result = vkCreateCommandPool(device.m_handle, &pool_info, nullptr, &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
}

auto VulkanCommandPool::deinit() -> void { vkDestroyCommandPool(m_device->m_handle, m_handle, nullptr); }

auto VulkanCommandPool::allocate_command_buffers(u32 amount) const -> std::vector<VulkanCommandBuffer> {
    VkResult result;

    std::vector<VkCommandBuffer>      handles(amount);
    const VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_handle,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<u32>(handles.size()),
    };
    result = vkAllocateCommandBuffers(m_device->m_handle, &alloc_info, handles.data());
    JF_ASSERT(result == VK_SUCCESS, "");


    std::vector<VulkanCommandBuffer> command_buffers(handles.size());
    for (u32 i = 0; i < command_buffers.size(); i++) {
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
} // namespace JadeFrame
