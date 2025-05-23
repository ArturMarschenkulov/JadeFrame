#include "command_buffer.h"

#include "JadeFrame/utils/assert.h"
#include "JadeFrame/utils/utils.h"

#include "logical_device.h"
#include "context.h"
#include "swapchain.h"
#include "pipeline.h"
#include "descriptor_set.h"
#include "physical_device.h"

namespace JadeFrame {

namespace vulkan {

/*---------------------------
    Command Buffer
---------------------------*/
CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
    : m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    , m_alloc_info(other.m_alloc_info)
    , m_device(std::exchange(other.m_device, nullptr))
    , m_command_pool(std::exchange(other.m_command_pool, nullptr))
    , m_stage(std::exchange(other.m_stage, STAGE::INVALID)) {}

auto CommandBuffer::operator=(CommandBuffer&& other) noexcept -> CommandBuffer& {
    m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
    m_alloc_info = other.m_alloc_info;
    m_device = std::exchange(other.m_device, nullptr);
    m_command_pool = std::exchange(other.m_command_pool, nullptr);
    m_stage = std::exchange(other.m_stage, STAGE::INVALID);

    return *this;
}

auto CommandBuffer::record_begin() -> void {

    const VkCommandBufferBeginInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    VkResult result = vkBeginCommandBuffer(m_handle, &info);
    JF_ASSERT(result == VK_SUCCESS, "");
    m_stage = STAGE::RECORDING;
}

auto CommandBuffer::record_end() -> void {
    VkResult result = vkEndCommandBuffer(m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
    m_stage = STAGE::EXCECUTABLE;
}

auto CommandBuffer::render_pass_begin(
    const Framebuffer& framebuffer,
    const RenderPass&  render_pass,
    const VkExtent2D&  extent,
    VkClearValue       clear_color
) -> void {
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");
    assert(
        m_level == LEVEL::PRIMARY &&
        "Only primary command buffers can begin a render pass"
    );

    std::array<VkClearValue, 2> clear_values = {
        clear_color, {1.0F, 0}
    };
    const VkRenderPassBeginInfo info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render_pass.m_handle,
        .framebuffer = framebuffer.m_handle,
        .renderArea =
            {
                         .offset = {0, 0},
                         .extent = extent,
                         },
        .clearValueCount = static_cast<u32>(clear_values.size()),
        .pClearValues = clear_values.data(),
    };

    vkCmdBeginRenderPass(m_handle, &info, VK_SUBPASS_CONTENTS_INLINE);
}

auto CommandBuffer::render_pass_end() -> void {
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");
    vkCmdEndRenderPass(m_handle);
}

auto CommandBuffer::reset() -> void {
    assert(
        m_stage == STAGE::EXCECUTABLE && "Command buffer must be in excecutable stage"
    );
    VkCommandBufferResetFlags flags = {};

    VkResult result = vkResetCommandBuffer(m_handle, flags);
    JF_ASSERT(result == VK_SUCCESS, "");
    m_stage = STAGE::INITIAL;
}

auto CommandBuffer::copy_buffer(const Buffer& src, const Buffer& dst, u64 size) const
    -> void {
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");

    // TODO: include bounds checking
    if (size == 0) { assert(false && "size is 0"); }
    if (size > src.m_size) { assert(false && "size is greater than src buffer size"); }
    if (size > dst.m_size) { assert(false && "size is greater than dst buffer size"); }

    const VkBufferCopy region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };
    vkCmdCopyBuffer(m_handle, src.m_handle, dst.m_handle, 1, &region);
}

auto CommandBuffer::copy_buffer_to_image(const Buffer& src, const Image& dst, v2u32 size)
    const -> void {
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");

    VkExtent3D extent = {
        .width = size.x,
        .height = size.y,
        .depth = 1,
    };
    VkOffset3D offset = {
        .x = 0,
        .y = 0,
        .z = 0,
    };
    VkImageSubresourceLayers subresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    const VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = subresource,
        .imageOffset = offset,
        .imageExtent = extent,
    };
    vkCmdCopyBufferToImage(
        m_handle,
        src.m_handle,
        dst.m_handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
}

auto CommandBuffer::bind_pipeline(
    const VkPipelineBindPoint bind_point,
    const Pipeline&           pipeline
) -> void {
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");

    vkCmdBindPipeline(
        m_handle,         // commandBuffer
        bind_point,       // pipelineBindPoint
        pipeline.m_handle // pipeline
    );
}

auto CommandBuffer::bind_vertex_buffers(
    u32                 first_binding,
    u32                 binding_count,
    const VkBuffer*     buffers,
    const VkDeviceSize* offsets
) -> void {
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");

    vkCmdBindVertexBuffers(
        m_handle,      // commandBuffer
        first_binding, // firstBinding
        binding_count, // bindingCount
        buffers,       // pBuffers
        offsets        // pOffsets
    );
}

auto CommandBuffer::bind_descriptor_sets(
    const VkPipelineBindPoint bind_point,
    const Pipeline&           pipeline,
    u32                       first_set,
    const DescriptorSet&      descriptor_set,
    const u32*                offset
) -> void {

    // JF_ASSERT(descriptor_set_count == descriptor_set.m_descriptors.size(), "");
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");

    vkCmdBindDescriptorSets(
        m_handle,                   // commandBuffer
        bind_point,                 // pipelineBindPoint
        pipeline.m_layout.m_handle, // layout
        first_set,                  // firstSet
        1, // descriptor_set.m_descriptors.size(),      // descriptorSetCount
        &descriptor_set.m_handle,                 // pDescriptorSets
        descriptor_set.m_layout->m_dynamic_count, // dynamicOffsetCount
        offset                                    // pDynamicOffsets
    );
}

auto CommandBuffer::bind_index_buffer(const Buffer& buffer, VkDeviceSize offset) -> void {
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");

    vkCmdBindIndexBuffer(
        m_handle,            // commandBuffer
        buffer.m_handle,     // buffer
        offset,              // offset
        VK_INDEX_TYPE_UINT32 // indexType
    );
}

auto CommandBuffer::draw(
    u32 vertex_count,
    u32 instance_count,
    u32 first_vertex,
    u32 first_instance
) -> void {
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");

    vkCmdDraw(
        m_handle,       // commandBuffer
        vertex_count,   // vertexCount
        instance_count, // instanceCount
        first_vertex,   // firstVertex
        first_instance  // firstInstance
    );
}

auto CommandBuffer::draw_indexed(
    u32 index_count,
    u32 instance_count,
    u32 first_index,
    u32 vertex_offset,
    u32 first_instance
) -> void {
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");

    vkCmdDrawIndexed(
        m_handle,                        // commandBuffer
        index_count,                     // indexCount
        instance_count,                  // instanceCount
        first_index,                     // firstIndex
        static_cast<i32>(vertex_offset), // vertexOffset
        first_instance                   // firstInstance
    );
}

static auto to_string_from_command_pool_create_flags(const VkCommandPoolCreateFlags& flag
) -> std::string {
    std::string result = "{ ";

    if (bit::check_flag(flag, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)) {
        result += "TRANSIENT ";
    }
    if (bit::check_flag(flag, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)) {
        result += "RESET_COMMAND_BUFFER ";
    }
    if (bit::check_flag(flag, VK_COMMAND_POOL_CREATE_PROTECTED_BIT)) {
        result += "PROTECTED ";
    }
    result += "}";
    return result;
}

/*---------------------------
    Command Pool
---------------------------*/

CommandPool::CommandPool(CommandPool&& other) noexcept
    : m_device(std::exchange(other.m_device, nullptr))
    , m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    , m_create_info(other.m_create_info) {}

auto CommandPool::operator=(CommandPool&& other) noexcept -> CommandPool& {
    if (this != &other) {
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
        m_create_info = std::exchange(other.m_create_info, {});
        m_device = std::exchange(other.m_device, nullptr);
    }
    return *this;
}

CommandPool::CommandPool(const LogicalDevice& device, QueueFamily& queue_family)
    : m_device(&device)
    , m_queue_family(&queue_family) {

    const VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // Optional
        .queueFamilyIndex =
            queue_family.m_index /*queue_family_indices.m_graphics_family.unwrap()*/,
    };

    VkResult result =
        vkCreateCommandPool(device.m_handle, &info, Instance::allocator(), &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
    {
        Logger::trace(
            "Created Command Pool {} at {}", fmt::ptr(this), fmt::ptr(m_handle)
        );
        Logger::trace("-flags: {}", to_string_from_command_pool_create_flags(info.flags));
        Logger::trace("-queueFamilyIndex: {}", info.queueFamilyIndex);
    }
    m_create_info = info;
}

CommandPool::~CommandPool() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device->m_handle, m_handle, Instance::allocator());
    }
}

auto CommandPool::allocate_buffers(u32 amount, CommandBuffer::LEVEL level) const
    -> std::vector<CommandBuffer> {

    VkCommandBufferLevel level_ = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    switch (level) {
        case CommandBuffer::LEVEL::PRIMARY: {
            level_ = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        } break;
        case CommandBuffer::LEVEL::SECONDARY: {
            level_ = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        } break;
    }

    std::vector<VkCommandBuffer>      handles(amount);
    const VkCommandBufferAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_handle,
        .level = level_,
        .commandBufferCount = static_cast<u32>(handles.size()),
    };
    VkResult result = vkAllocateCommandBuffers(m_device->m_handle, &info, handles.data());
    JF_ASSERT(result == VK_SUCCESS, "");
    {
        Logger::trace(
            "Allocated {} Command Buffers to {} from pool {}",
            amount,
            fmt::ptr(*handles.data()),
            fmt::ptr(m_handle)
        );
    }

    std::vector<CommandBuffer> buffers(handles.size());
    for (u32 i = 0; i < buffers.size(); i++) {
        buffers[i].m_handle = handles[i];
        buffers[i].m_alloc_info = info;
        buffers[i].m_device = m_device;
        buffers[i].m_command_pool = this;
        buffers[i].m_stage = CommandBuffer::STAGE::INITIAL;
        buffers[i].m_level = level;
    }
    return buffers;
}

auto CommandPool::allocate_buffer() const -> CommandBuffer {
    CommandBuffer::LEVEL level = CommandBuffer::LEVEL::PRIMARY;
    return std::move(this->allocate_buffers(1, level)[0]);
}

auto CommandPool::free_buffers(const std::span<CommandBuffer>& command_buffers
) const -> void {
    for (u32 i = 0; i < command_buffers.size(); i++) {
        vkFreeCommandBuffers(
            m_device->m_handle, m_handle, 1, &command_buffers[i].m_handle
        );
        {
            Logger::trace(
                "Freed Command Buffer {} from {}",
                fmt::ptr(command_buffers[i].m_handle),
                fmt::ptr(m_handle)
            );
        }
    }
}

auto CommandPool::free_buffer(const CommandBuffer& command_buffer) const -> void {
    vkFreeCommandBuffers(m_device->m_handle, m_handle, 1, &command_buffer.m_handle);
    {
        Logger::trace(
            "Freed Command Buffer {} from {}",
            fmt::ptr(command_buffer.m_handle),
            fmt::ptr(m_handle)
        );
    }
}

auto CommandPool::copy_buffer(
    const Buffer& src_buffer,
    const Buffer& dst_buffer,
    VkDeviceSize  size
) const -> void {

    CommandBuffer cmd = this->allocate_buffer();

    cmd.record([&] { cmd.copy_buffer(src_buffer, dst_buffer, size); });

    // TODO: the queue should be provided by the user. It does not make sense for the
    // command pool to decide which queue to use.
    cmd.m_device->m_graphics_queue.submit(cmd);
    cmd.m_device->m_graphics_queue.wait_idle();

    this->free_buffer(cmd);
}

auto CommandPool::transition_layout(
    const Image& image,
    VkFormat /*format*/,
    VkImageLayout old_layout,
    VkImageLayout new_layout
    // VkImageSubresourceRange subresource_range
) const -> void {
    auto cmd = this->allocate_buffer();

    cmd.record([&] {
        VkImageMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = old_layout,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image.m_handle,
            .subresourceRange =
                {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                   .baseMipLevel = 0,
                                   .levelCount = 1,
                                   .baseArrayLayer = 0,
                                   .layerCount = 1},
        };

        VkPipelineStageFlags source_stage = {};
        VkPipelineStageFlags destination_stage = {};

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            assert(false);
        }

        vkCmdPipelineBarrier(
            cmd.m_handle,
            source_stage,
            destination_stage,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );
    });
    // cb[0].record_end();

    cmd.m_device->m_graphics_queue.submit(cmd);
    cmd.m_device->m_graphics_queue.wait_idle();

    this->free_buffer(cmd);
}

auto CommandPool::copy_buffer_to_image(
    const Buffer& buffer,
    const Image&  image,
    v2u32         size
) const -> void {

    const CommandPool& cmd_pool = m_device->m_command_pool;
    auto               cmd = cmd_pool.allocate_buffer();

    cmd.record([&] { cmd.copy_buffer_to_image(buffer, image, size); });
    cmd.m_device->m_graphics_queue.submit(cmd);
    cmd.m_device->m_graphics_queue.wait_idle();

    cmd_pool.free_buffer(cmd);
}

auto CommandBuffer::execute_command(const CommandBuffer& command_buffer) -> void {
    assert(m_stage == STAGE::RECORDING && "Command buffer must be in recording stage");

    vkCmdExecuteCommands(m_handle, 1, &command_buffer.m_handle);
}
} // namespace vulkan
} // namespace JadeFrame
