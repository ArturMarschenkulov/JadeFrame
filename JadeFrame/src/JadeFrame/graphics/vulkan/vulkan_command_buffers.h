#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/prelude.h"

#include <vector>


namespace JadeFrame {




namespace vulkan {
class QueueFamilyIndices;
class RenderPass;
class Framebuffer;
class LogicalDevice;
class DescriptorSet;
class Swapchain;
class Buffer;
class Pipeline;
class CommandPool;

class CommandBuffer {
public:
    auto record_begin() -> void;
    auto record_end() -> void;
    auto render_pass_begin(
        const Framebuffer& framebuffer, const RenderPass& render_pass, const VkExtent2D& swapchain, VkClearValue color)
        -> void;
    auto render_pass_end() -> void;

    auto reset() -> void;

    auto copy_buffer(const Buffer& src, const Buffer& dst, u32 region_size, VkBufferCopy* regions) -> void;

public:
    VkCommandBuffer      m_handle;
    const LogicalDevice* m_device = nullptr;
    const CommandPool*   m_command_pool = nullptr;
};

using QueueFamilyIndex = u32;
class CommandPool {
public:
    auto init(const LogicalDevice& device, const QueueFamilyIndex& queue_family_index) -> void;
    auto deinit() -> void;

    auto allocate_command_buffers(u32 amount) const -> std::vector<CommandBuffer>;
    auto allocate_command_buffer() const -> CommandBuffer;
    auto free_command_buffers(const std::vector<CommandBuffer>& command_buffers) const -> void;

public:
    const LogicalDevice* m_device = nullptr;
    VkCommandPool        m_handle = VK_NULL_HANDLE;
};

} // namespace vulkan
} // namespace JadeFrame