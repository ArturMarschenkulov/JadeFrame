#include "pch.h"
#include "vulkan_logical_device.h"
#include "vulkan_context.h"
#include "vulkan_command_buffers.h"
#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_physical_device.h"
#include "JadeFrame/utils/assert.h"

namespace JadeFrame {

namespace vulkan {

/*---------------------------
    Command Buffer
---------------------------*/
CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
    : m_handle(other.m_handle)
    , m_alloc_info(other.m_alloc_info)
    , m_device(other.m_device)
    , m_command_pool(other.m_command_pool)
    , m_stage(other.m_stage) {

    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_command_pool = nullptr;
    other.m_stage = STAGE::INVALID;
}

auto CommandBuffer::operator=(CommandBuffer&& other) noexcept -> CommandBuffer& {
    this->m_handle = other.m_handle;
    this->m_alloc_info = other.m_alloc_info;
    this->m_device = other.m_device;
    this->m_command_pool = other.m_command_pool;
    this->m_stage = other.m_stage;

    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_command_pool = nullptr;
    other.m_stage = STAGE::INVALID;
    return *this;
}

auto CommandBuffer::record_begin() -> void {

    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    VkResult result = vkBeginCommandBuffer(m_handle, &begin_info);
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
    const VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
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

auto CommandBuffer::render_pass_end() -> void { vkCmdEndRenderPass(m_handle); }

auto CommandBuffer::reset() -> void {

    VkCommandBufferResetFlags flags = {};

    VkResult result = vkResetCommandBuffer(m_handle, flags);
    JF_ASSERT(result == VK_SUCCESS, "");
}

auto CommandBuffer::copy_buffer(const Buffer& src, const Buffer& dst, u64 size) const
    -> void {

    // TODO: include bounds checking

    const VkBufferCopy region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };
    vkCmdCopyBuffer(m_handle, src.m_handle, dst.m_handle, 1, &region);
}

auto CommandBuffer::copy_buffer_to_image(const Buffer& src, const Image& dst, v2u32 size)
    const -> void {
    const VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                               .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                               .mipLevel = 0,
                               .baseArrayLayer = 0,
                               .layerCount = 1,
                               },
        .imageOffset = {                                      0,           0,    0 },
        .imageExtent = {                             size.width, size.height,    1 },
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
    vkCmdDrawIndexed(
        m_handle,                        // commandBuffer
        index_count,                     // indexCount
        instance_count,                  // instanceCount
        first_index,                     // firstIndex
        static_cast<i32>(vertex_offset), // vertexOffset
        first_instance                   // firstInstance
    );
}

static auto to_string_from_command_pool_create_flags(const VkCommandPoolCreateFlags& flag)
    -> std::string {
    std::string result = "{ ";
    if ((flag & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) != 0U) { result += "TRANSIENT "; }
    if ((flag & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) != 0U) {
        result += "RESET_COMMAND_BUFFER ";
    }
    if ((flag & VK_COMMAND_POOL_CREATE_PROTECTED_BIT) != 0U) { result += "PROTECTED "; }
    result += "}";
    return result;
}

/*---------------------------
    Command Pool
---------------------------*/

CommandPool::CommandPool(CommandPool&& other) noexcept
    : m_device(other.m_device)
    , m_handle(other.m_handle)
    , m_create_info(other.m_create_info) {

    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
}

auto CommandPool::operator=(CommandPool&& other) noexcept -> CommandPool& {
    if (this != &other) {
        this->m_handle = other.m_handle;
        this->m_create_info = other.m_create_info;
        this->m_device = other.m_device;

        other.m_handle = VK_NULL_HANDLE;
        other.m_create_info = {};
        other.m_device = nullptr;
    }
    return *this;
}

CommandPool::CommandPool(
    const LogicalDevice&    device,
    const QueueFamilyIndex& queue_family_index
)
    : m_device(&device) {

    const VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // Optional
        .queueFamilyIndex =
            queue_family_index /*queue_family_indices.m_graphics_family.unwrap()*/,
    };

    VkResult result = vkCreateCommandPool(
        device.m_handle, &pool_info, Instance::allocator(), &m_handle
    );
    JF_ASSERT(result == VK_SUCCESS, "");
    {
        Logger::trace(
            "Created Command Pool {} at {}", fmt::ptr(this), fmt::ptr(m_handle)
        );
        Logger::trace(
            "-flags: {}", to_string_from_command_pool_create_flags(pool_info.flags)
        );
        Logger::trace("-queueFamilyIndex: {}", pool_info.queueFamilyIndex);
    }
    m_create_info = pool_info;
}

CommandPool::~CommandPool() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device->m_handle, m_handle, Instance::allocator());
    }
}

auto CommandPool::allocate_buffers(u32 amount) const -> std::vector<CommandBuffer> {

    std::vector<VkCommandBuffer>      handles(amount);
    const VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_handle,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<u32>(handles.size()),
    };
    VkResult result =
        vkAllocateCommandBuffers(m_device->m_handle, &alloc_info, handles.data());
    JF_ASSERT(result == VK_SUCCESS, "");
    {
        Logger::trace(
            "Allocated {} Command Buffers to {} from pool {}",
            amount,
            fmt::ptr(*handles.data()),
            fmt::ptr(m_handle)
        );
    }

    std::vector<CommandBuffer> command_buffers(handles.size());
    for (u32 i = 0; i < command_buffers.size(); i++) {
        command_buffers[i].m_handle = handles[i];
        command_buffers[i].m_alloc_info = alloc_info;
        command_buffers[i].m_device = m_device;
        command_buffers[i].m_command_pool = this;
        command_buffers[i].m_stage = CommandBuffer::STAGE::INITIAL;
    }
    return command_buffers;
}

auto CommandPool::allocate_buffer() const -> CommandBuffer {
    return std::move(this->allocate_buffers(1)[0]);
}

auto CommandPool::free_buffers(const std::vector<CommandBuffer>& command_buffers) const
    -> void {
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
} // namespace vulkan
} // namespace JadeFrame
