#include "pch.h"
#include "vulkan_buffer.h"

#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_context.h"

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/utils/assert.h"
#include "JadeFrame/utils/utils.h"

namespace JadeFrame {

namespace vulkan {
struct Memory {
    struct Block {
        VkDeviceMemory       memory;
        u32                  size;
        VkMemoryRequirements mem_reqs;
    };

    auto push_buffer(const LogicalDevice* device, VkBuffer buffer) -> void {
        m_device = device;
        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(m_device->m_handle, buffer, &mem_reqs);

        if (m_blocks.empty()) {
            auto default_size = from_mebibyte(256);

            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.pNext = nullptr;
            alloc_info.allocationSize =
                default_size <= mem_reqs.size ? mem_reqs.size : default_size;
            alloc_info.memoryTypeIndex = m_device->m_physical_device->find_memory_type(
                mem_reqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );

            VkDeviceMemory memory;
            vkAllocateMemory(
                m_device->m_handle, &alloc_info, Instance::allocator(), &memory
            );
            m_blocks.push_back(memory);
        }
    }

    const LogicalDevice*        m_device;
    std::vector<VkDeviceMemory> m_blocks;
};

static Memory g_memory;

/*---------------------------
    Buffer
---------------------------*/

auto to_string(const Buffer::TYPE type) -> const char* {
    switch (type) {
        case Buffer::TYPE::VERTEX: return "VERTEX";
        case Buffer::TYPE::INDEX: return "INDEX";
        case Buffer::TYPE::UNIFORM: return "UNIFORM";
        case Buffer::TYPE::STAGING: return "STAGING";
        case Buffer::TYPE::UNINIT: return "UNINIT";
        default: return "UNKNOWN";
    }
}

static auto does_use_staging_buffer(const Buffer::TYPE type) -> bool {
    bool result = false;
    switch (type) {
        case Buffer::TYPE::VERTEX: result = true; break;
        case Buffer::TYPE::INDEX: result = true; break;
        case Buffer::TYPE::UNIFORM: result = false; break;
        case Buffer::TYPE::STAGING: result = false; break;
        case Buffer::TYPE::UNINIT: result = false; break;
        default: JF_ASSERT(false, ""); break;
    }
    return result;
}

static auto get_usage(const Buffer::TYPE type) -> VkBufferUsageFlags {
    VkBufferUsageFlags result = {};
    switch (type) {
        case Buffer::TYPE::VERTEX:
            result = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case Buffer::TYPE::INDEX:
            result = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case Buffer::TYPE::UNIFORM: result = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
        case Buffer::TYPE::STAGING: result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; break;
        case Buffer::TYPE::UNINIT: result = 0; break;
        default: JF_ASSERT(false, ""); break;
    }
    return result;
}

static auto get_properties(const Buffer::TYPE type) -> VkMemoryPropertyFlags {
    VkMemoryPropertyFlags result = {};
    switch (type) {
        case Buffer::TYPE::VERTEX:
        case Buffer::TYPE::INDEX: result = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; break;
        case Buffer::TYPE::UNIFORM:
        case Buffer::TYPE::STAGING:
            result = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        case Buffer::TYPE::UNINIT: result = 0; break;
        default: JF_ASSERT(false, ""); break;
    }
    return result;
}

Buffer::Buffer(Buffer&& other)
    : m_type(other.m_type)
    , m_size(other.m_size)
    , m_handle(other.m_handle)
    , m_memory(other.m_memory)
    , m_device(other.m_device) {

    other.m_type = TYPE::UNINIT;
    other.m_size = 0;
    other.m_handle = VK_NULL_HANDLE;
    other.m_memory = VK_NULL_HANDLE;
    other.m_device = nullptr;
}

auto Buffer::operator=(Buffer&& other) -> Buffer& {
    m_type = other.m_type;
    m_size = other.m_size;
    m_handle = other.m_handle;
    m_memory = other.m_memory;
    m_device = other.m_device;

    other.m_type = TYPE::UNINIT;
    other.m_size = 0;
    other.m_handle = VK_NULL_HANDLE;
    other.m_memory = VK_NULL_HANDLE;
    other.m_device = nullptr;
    return *this;
}

Buffer::Buffer(
    const LogicalDevice& device,
    Buffer::TYPE         buffer_type,
    void*                data,
    size_t               size
) {
    /*VkResult result;*/
    m_device = &device;
    m_size = size;
    m_type = buffer_type;

    bool                  b_with_staging_buffer = does_use_staging_buffer(buffer_type);
    VkBufferUsageFlags    usage = get_usage(buffer_type);
    VkMemoryPropertyFlags properties = get_properties(buffer_type);

    if (b_with_staging_buffer == true) {
        Buffer* staging_buffer =
            device.create_buffer(Buffer::TYPE::STAGING, nullptr, size);
        staging_buffer->write(data, 0, size);

        this->create_buffer(size, usage, properties, m_handle, m_memory);
        this->copy_buffer(*staging_buffer, *this, size);
    } else {
        assert(data == nullptr);
        this->create_buffer(size, usage, properties, m_handle, m_memory);
    }
}

Buffer::~Buffer() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->m_handle, m_handle, Instance::allocator());
        vkFreeMemory(m_device->m_handle, m_memory, Instance::allocator());

        Logger::trace("Destroyed Buffer {} at {}", fmt::ptr(this), fmt::ptr(m_handle));

        m_handle = VK_NULL_HANDLE;
        m_memory = VK_NULL_HANDLE;
    }
}

auto Buffer::write(const Matrix4x4& m, VkDeviceSize offset) -> void {
    this->write((void*)&m, offset, sizeof(m));
}

auto Buffer::write(void* data, VkDeviceSize offset, VkDeviceSize size) -> void {
    VkResult result;

    void* mapped_data;
    result = vkMapMemory(m_device->m_handle, m_memory, offset, size, 0, &mapped_data);
    JF_ASSERT(result == VK_SUCCESS, "");
    memcpy(mapped_data, data, static_cast<size_t>(size));
    vkUnmapMemory(m_device->m_handle, m_memory);
}

auto Buffer::resize(size_t size) -> void {
    assert(m_type == TYPE::UNIFORM);
    if (size == m_size) { return; }

    this->~Buffer();
    *this = Buffer(*m_device, m_type, nullptr, size);
}

auto Buffer::create_buffer(
    VkDeviceSize          size,
    VkBufferUsageFlags    usage,
    VkMemoryPropertyFlags properties,
    VkBuffer&             buffer,
    VkDeviceMemory&       buffer_memory
) -> void {
    VkResult result;

    const VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };
    result =
        vkCreateBuffer(m_device->m_handle, &buffer_info, Instance::allocator(), &buffer);
    JF_ASSERT(result == VK_SUCCESS, "");
    m_handle = buffer;

    g_memory.push_buffer(m_device, buffer);

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(m_device->m_handle, buffer, &mem_requirements);

    const VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = m_device->m_physical_device->find_memory_type(
            mem_requirements.memoryTypeBits, properties
        ),
    };

    result = vkAllocateMemory(
        m_device->m_handle, &alloc_info, Instance::allocator(), &buffer_memory
    );
    JF_ASSERT(result == VK_SUCCESS, "");

    result = vkBindBufferMemory(m_device->m_handle, buffer, buffer_memory, 0);
    { Logger::trace("Created Buffer {} at {}", fmt::ptr(this), fmt::ptr(m_handle)); }
}

auto Buffer::copy_buffer(
    const Buffer& src_buffer,
    const Buffer& dst_buffer,
    VkDeviceSize  size
) -> void {
    const CommandPool& cmd_pool = m_device->m_command_pool;

    CommandBuffer cmd = cmd_pool.allocate_buffer();

    cmd.record([&] {
        const VkBufferCopy copy_region = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };
        vkCmdCopyBuffer(
            cmd.m_handle, src_buffer.m_handle, dst_buffer.m_handle, 1, &copy_region
        );
    });
    // buffer[0].record_end();

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = {},
        .waitSemaphoreCount = {},
        .pWaitSemaphores = {},
        .pWaitDstStageMask = {},
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd.m_handle,
        .signalSemaphoreCount = {},
        .pSignalSemaphores = {},
    };

    m_device->m_graphics_queue.submit(submit_info, nullptr);
    m_device->m_graphics_queue.wait_idle();

    cmd_pool.free_buffer(cmd);
}

GPUMeshData::GPUMeshData(
    const LogicalDevice& device,
    const VertexData&    vertex_data,
    const VertexFormat,
    bool interleaved
) {

    const std::vector<f32> flat_data = convert_into_data(vertex_data, interleaved);

    void*  data = (void*)flat_data.data();
    size_t size = sizeof(flat_data[0]) * flat_data.size();
    m_vertex_buffer = device.create_buffer(Buffer::TYPE::VERTEX, data, size);

    if (vertex_data.m_indices.size() > 0) {
        auto&  indices = vertex_data.m_indices;
        void*  indices_data = (void*)indices.data();
        size_t indices_size = sizeof(indices[0]) * indices.size();
        m_index_buffer =
            device.create_buffer(Buffer::TYPE::INDEX, indices_data, indices_size);
    }
}

auto GPUMeshData::bind() const -> void {}

auto GPUMeshData::set_layout(const VertexFormat& /*vertex_format*/) -> void {}

/*---------------------------
    Image
---------------------------*/

Image::Image(Image&& other) noexcept
    : m_handle(other.m_handle)
    , m_device(other.m_device)
    , m_memory(other.m_memory)
    , m_source(other.m_source)
    , m_size(other.m_size) {
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_memory = VK_NULL_HANDLE;
    other.m_source = SOURCE::REGULAR;
    other.m_size = {0, 0};
}

auto Image::operator=(Image&& other) noexcept -> Image& {
    if (this != &other) {
        m_handle = other.m_handle;
        m_device = other.m_device;
        m_memory = other.m_memory;
        m_source = other.m_source;
        m_size = other.m_size;
        other.m_handle = VK_NULL_HANDLE;
        other.m_device = nullptr;
        other.m_memory = VK_NULL_HANDLE;
        other.m_source = SOURCE::REGULAR;
        other.m_size = {0, 0};
    }
    return *this;
}

Image::~Image() {
    if (m_handle != VK_NULL_HANDLE) {
        switch (m_source) {
            case SOURCE::REGULAR: {
                JF_ASSERT(false, "");
            } break;
            case SOURCE::SWAPCHAIN: {

            } break;
            default: JF_ASSERT(false, "");
        }
    }
}

Image::Image(
    const LogicalDevice& device,
    const v2u32&         size,
    VkFormat             format,
    VkImageUsageFlags    usage
) {
    m_device = &device;
    m_source = SOURCE::REGULAR;
    m_size = size;
    VkImageFormatProperties props;

    VkResult result;
    result = vkGetPhysicalDeviceImageFormatProperties(
        device.m_physical_device->m_handle,
        format,
        VK_IMAGE_TYPE_2D,
        VK_IMAGE_TILING_OPTIMAL,
        usage,
        0,
        &props
    );

    const VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {.width = size.width, .height = size.height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    result =
        vkCreateImage(device.m_handle, &image_info, Instance::allocator(), &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
    {
        Logger::trace("Created image {} at {}", fmt::ptr(this), fmt::ptr(m_handle));
        Logger::trace(
            "-w, h, d: {}, {}, {}",
            image_info.extent.width,
            image_info.extent.height,
            image_info.extent.depth
        );
        Logger::trace("-mip levels: {}", image_info.mipLevels);
        Logger::trace("-array layers: {}", image_info.arrayLayers);
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device.m_handle, m_handle, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = device.m_physical_device->find_memory_type(
            mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        ),
    };

    result =
        vkAllocateMemory(device.m_handle, &alloc_info, Instance::allocator(), &m_memory);
    JF_ASSERT(result == VK_SUCCESS, "");

    result = vkBindImageMemory(device.m_handle, m_handle, m_memory, 0);
    JF_ASSERT(result == VK_SUCCESS, "");
}

Image::Image(const LogicalDevice& device, VkImage image) {
    m_device = &device;
    m_handle = image;
    m_source = SOURCE::SWAPCHAIN;
}

/*---------------------------
    Image View
---------------------------*/

ImageView::ImageView(ImageView&& other) noexcept
    : m_handle(other.m_handle)
    , m_device(other.m_device)
    , m_image(other.m_image) {
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_image = nullptr;
}

auto ImageView::operator=(ImageView&& other) noexcept -> ImageView& {
    if (this != &other) {
        m_handle = other.m_handle;
        m_device = other.m_device;
        m_image = other.m_image;
        other.m_handle = VK_NULL_HANDLE;
        other.m_device = nullptr;
        other.m_image = nullptr;
    }
    return *this;
}

ImageView::ImageView(const LogicalDevice& device, const Image& image, VkFormat format) {
    m_device = &device;
    m_image = &image;
    VkResult              result;
    VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = image.m_handle,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components =
            {
                         .r = VK_COMPONENT_SWIZZLE_R,
                         .g = VK_COMPONENT_SWIZZLE_G,
                         .b = VK_COMPONENT_SWIZZLE_B,
                         .a = VK_COMPONENT_SWIZZLE_A,
                         },
        .subresourceRange = {
                         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                         .baseMipLevel = 0,
                         .levelCount = 1,
                         .baseArrayLayer = 0,
                         .layerCount = 1,
                         }
    };

    result = vkCreateImageView(
        device.m_handle, &create_info, Instance::allocator(), &m_handle
    );
    if (result != VK_SUCCESS) { assert(false); }
}

ImageView::~ImageView() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device->m_handle, m_handle, Instance::allocator());
    }
}

/*---------------------------
        Sampler
---------------------------*/

auto Sampler::init(const LogicalDevice& device) -> void {
    m_device = &device;
    VkResult result;

    const VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable =
            device.m_physical_device->m_features.samplerAnisotropy == VK_TRUE ? VK_TRUE
                                                                              : VK_FALSE,
        .maxAnisotropy =
            device.m_physical_device->m_properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    result =
        vkCreateSampler(device.m_handle, &samplerInfo, Instance::allocator(), &m_handle);
    if (result != VK_SUCCESS) { assert(false); }
}

auto Sampler::deinit() -> void {}

/*---------------------------
        Texture
---------------------------*/

Vulkan_Texture::Vulkan_Texture(
    const LogicalDevice& device,
    void*                data,
    v2u32                size,
    VkFormat             format
) {
    m_device = &device;
    u32 comp_count = 0;
    if (format == VK_FORMAT_R8G8B8_SRGB) {
        comp_count = 3;
    } else if (format == VK_FORMAT_R8G8B8A8_SRGB) {
        comp_count = 4;
    } else {
        assert(false);
    }
    VkDeviceSize image_size = size.width * size.height * comp_count;
    Buffer       staging_buffer(device, Buffer::TYPE::STAGING, nullptr, image_size);
    staging_buffer.write(data, 0, image_size);

    // Image image;
    m_image = Image(
        device, size, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    );

    this->transition_layout(
        m_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );
    this->copy_buffer_to_image(staging_buffer, m_image, size);
    this->transition_layout(
        m_image,
        format,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    m_sampler.init(device);
}

auto Vulkan_Texture::deinit() -> void {}

auto Vulkan_Texture::transition_layout(
    const Image& image,
    VkFormat /*format*/,
    VkImageLayout old_layout,
    VkImageLayout new_layout
) -> void {
    const LogicalDevice& d = *m_device;

    auto cb = d.m_command_pool.allocate_buffer();

    cb.record([&] {
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
        } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            assert(false);
        }

        vkCmdPipelineBarrier(
            cb.m_handle,
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

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &cb.m_handle,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };
    d.m_graphics_queue.submit(submit_info, VK_NULL_HANDLE);
    d.m_graphics_queue.wait_idle();

    d.m_command_pool.free_buffer(cb);
}

auto Vulkan_Texture::copy_buffer_to_image(
    const Buffer& buffer,
    const Image&  image,
    v2u32         size
) -> void {
    const LogicalDevice& d = *m_device;

    auto cb = d.m_command_pool.allocate_buffer();
    cb.record([&] {
        const VkBufferImageCopy region = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                   .mipLevel = 0,
                                   .baseArrayLayer = 0,
                                   .layerCount = 1},
            .imageOffset = {0, 0, 0},
            .imageExtent = {size.width, size.height, 1},
        };

        vkCmdCopyBufferToImage(
            cb.m_handle,
            buffer.m_handle,
            image.m_handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    });
    // cb[0].record_end();

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &cb.m_handle,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };
    // d.m_graphics_queue.submit(submit_info, VK_NULL_HANDLE);
    d.m_graphics_queue.submit(cb, nullptr, nullptr, nullptr);
    d.m_graphics_queue.wait_idle();

    d.m_command_pool.free_buffer(cb);
}
} // namespace vulkan
} // namespace JadeFrame