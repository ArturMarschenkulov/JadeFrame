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

    template<typename Func>
    auto record(Func&& func) -> void {
        record_begin();
        func();
        record_end();
    }
    template<typename Func>
    auto render_pass(
        const Framebuffer& framebuffer, const RenderPass& render_pass, const VkExtent2D& swapchain, VkClearValue color,
        Func&& func) -> void {
        render_pass_begin(framebuffer, render_pass, swapchain, color);
        func();
        render_pass_end();
    }

    auto reset() -> void;

    auto copy_buffer(const Buffer& src, const Buffer& dst, u32 region_size, VkBufferCopy* regions) -> void;


    auto bind_pipeline(const VkPipelineBindPoint bind_point, const Pipeline& pipeline) -> void;
    auto bind_vertex_buffers(u32 first_binding, u32 binding_count, const VkBuffer* buffers, const VkDeviceSize* offsets)
        -> void;
    auto bind_vertex_buffers(const VkBuffer* buffers, const VkDeviceSize* offsets) -> void;
    auto bind_descriptor_sets(
        const VkPipelineBindPoint bind_point, const Pipeline& pipeline, u32 first_set,
        const DescriptorSet& descriptor_set, const u32* offset) -> void;


    auto bind_index_buffer(const Buffer& buffer, VkDeviceSize offset) -> void;

    auto draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) -> void;
    auto draw_indexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance)
        -> void;

public:
    enum class STAGE {
        INITIAL,
        RECORDING,
        EXCECUTABLE,
        PENDING,
        INVALID
    };
    VkCommandBuffer             m_handle;
    VkCommandBufferAllocateInfo m_alloc_info;
    const LogicalDevice*        m_device = nullptr;
    const CommandPool*          m_command_pool = nullptr;
    mutable STAGE               m_stage = STAGE::INVALID;
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
    const LogicalDevice*    m_device = nullptr;
    VkCommandPool           m_handle = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo m_create_info = {};
};

} // namespace vulkan
} // namespace JadeFrame