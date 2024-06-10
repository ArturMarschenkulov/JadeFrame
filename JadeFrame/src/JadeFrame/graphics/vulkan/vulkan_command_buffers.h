#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/graphics/vulkan/vulkan_buffer.h"
#include "JadeFrame/prelude.h"

#include <vector>

namespace JadeFrame {

namespace vulkan {
class RenderPass;
class Framebuffer;
class LogicalDevice;
class DescriptorSet;
class Swapchain;
class Buffer;
class Image;
class Pipeline;
class CommandPool;

class CommandBuffer {
public:
    CommandBuffer() = default;
    ~CommandBuffer() = default;
    CommandBuffer(const CommandBuffer&) = delete;
    auto operator=(const CommandBuffer&) -> CommandBuffer& = delete;
    CommandBuffer(CommandBuffer&& other) noexcept;
    auto operator=(CommandBuffer&& other) noexcept -> CommandBuffer&;

    auto record_begin() -> void;
    auto record_end() -> void;
    auto render_pass_begin(
        const Framebuffer& framebuffer,
        const RenderPass&  render_pass,
        const VkExtent2D&  swapchain,
        VkClearValue       color
    ) -> void;
    auto render_pass_end() -> void;

    template<typename Func>
    auto record(Func&& func) -> void {
        this->record_begin();
        func();
        this->record_end();
    }

    template<typename Func>
    auto render_pass(
        const Framebuffer& framebuffer,
        const RenderPass&  render_pass,
        const VkExtent2D&  swapchain,
        VkClearValue       color,
        Func&&             func
    ) -> void {
        this->render_pass_begin(framebuffer, render_pass, swapchain, color);
        func();
        this->render_pass_end();
    }

    auto reset() -> void;

public: // copy methods
    auto copy_buffer(const Buffer& src, const Buffer& dst, u64 size) const -> void;
    auto copy_buffer_to_image(const Buffer& src, const Image& dst, v2u32 size) const
        -> void;

public: // bind methods
    auto bind_pipeline(const VkPipelineBindPoint bind_point, const Pipeline& pipeline)
        -> void;
    auto bind_vertex_buffers(
        u32                 first_binding,
        u32                 binding_count,
        const VkBuffer*     buffers,
        const VkDeviceSize* offsets
    ) -> void;
    auto bind_vertex_buffers(const VkBuffer* buffers, const VkDeviceSize* offsets)
        -> void;

    auto bind_vertex_buffer(u32 binding, const Buffer& buffer, const VkDeviceSize& offset)
        -> void {
        this->bind_vertex_buffers(binding, 1, &buffer.m_handle, &offset);
    }

    auto bind_descriptor_sets(
        const VkPipelineBindPoint bind_point,
        const Pipeline&           pipeline,
        u32                       first_set,
        const DescriptorSet&      descriptor_set,
        const u32*                offset
    ) -> void;

    auto bind_index_buffer(const Buffer& buffer, VkDeviceSize offset) -> void;

public: // draw methods
    auto draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
        -> void;
    auto draw_indexed(
        u32 index_count,
        u32 instance_count,
        u32 first_index,
        u32 vertex_offset,
        u32 first_instance
    ) -> void;

public:
    auto execute_command(const CommandBuffer& command_buffer) -> void;

public:
    enum class STAGE {
        INITIAL,     // after vkBegineCommandBuffer, before vkCmdBeginRenderPass
        RECORDING,   // after vkCmdBeginRenderPass, before vkCmdEndRenderPass
        EXCECUTABLE, // after vkEndCommandBuffer, before vkQueueSubmit
        PENDING,
        INVALID
    };
    enum class LEVEL {
        PRIMARY,
        SECONDARY
    };
    VkCommandBuffer             m_handle;
    VkCommandBufferAllocateInfo m_alloc_info;
    const LogicalDevice*        m_device = nullptr;
    const CommandPool*          m_command_pool = nullptr;
    mutable STAGE               m_stage = STAGE::INVALID;
    LEVEL                       m_level = LEVEL::PRIMARY;
};

using QueueFamilyIndex = u32;

class CommandPool {
public:
    CommandPool() = default;
    ~CommandPool();
    CommandPool(const CommandPool&) = delete;
    auto operator=(const CommandPool&) -> CommandPool& = delete;
    CommandPool(CommandPool&& other) noexcept;
    auto operator=(CommandPool&& other) noexcept -> CommandPool&;

public:
    CommandPool(const LogicalDevice& device, const QueueFamilyIndex& queue_family_index);

public:
    [[nodiscard]] auto allocate_buffers(u32 amount, CommandBuffer::LEVEL level) const
        -> std::vector<CommandBuffer>;
    [[nodiscard]] auto allocate_buffer() const -> CommandBuffer;
    auto free_buffers(const std::vector<CommandBuffer>& command_buffers) const -> void;
    auto free_buffer(const CommandBuffer& command_buffer) const -> void;

    auto
    copy_buffer(const Buffer& src_buffer, const Buffer& dst_buffer, VkDeviceSize size)
        const -> void;

public:
    const LogicalDevice*    m_device = nullptr;
    VkCommandPool           m_handle = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo m_create_info = {};
};

} // namespace vulkan
} // namespace JadeFrame