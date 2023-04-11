#include "pch.h"
#include "vulkan_buffer.h"

#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/utils/assert.h"

namespace JadeFrame {

namespace vulkan {

auto LogicalDevice::create_texture_image(const std::string& path) -> void {
    /*VkResult result;*/

    JadeFrame::Image image = JadeFrame::Image::load(path);


    if (image.data) {
        VkDeviceSize image_size = image.width * image.height * image.num_components;

        vulkan::Buffer staging_buffer(*this, vulkan::Buffer::TYPE::STAGING, nullptr, image_size);

        staging_buffer.write(image.data, 0, image_size);

        this->create_image(
            {static_cast<u32>(image.width), static_cast<u32>(image.height)}, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_texture_image.m_handle, m_texture_image_memory);

        // this->transition_image_layout(
        //	m_texture_image.m_handle,
        //	VK_FORMAT_R8G8B8A8_SRGB,
        //	VK_IMAGE_LAYOUT_UNDEFINED,
        //	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        //);
        // this->copy_buffer_to_image(staging_buffer.m_handle, m_texture_image.m_handle,
        // static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height)); this->transition_image_layout(
        //	m_texture_image.m_handle,
        //	VK_FORMAT_R8G8B8A8_SRGB,
        //	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        //);

        staging_buffer.deinit();
    } else {
        JF_ASSERT(false, "");
    }
}
auto LogicalDevice::create_image(
    v2u32 size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    VkImage& image, VkDeviceMemory& image_memory) -> void {
    VkResult                result;
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
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    result = vkCreateImage(m_handle, &image_info, nullptr, &image);
    JF_ASSERT(result == VK_SUCCESS, "");


    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(m_handle, image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = m_physical_device->find_memory_type(mem_requirements.memoryTypeBits, properties),
    };
    result = vkAllocateMemory(m_handle, &alloc_info, nullptr, &image_memory);
    JF_ASSERT(result == VK_SUCCESS, "");

    result = vkBindImageMemory(m_handle, image, image_memory, 0);
    JF_ASSERT(result == VK_SUCCESS, "");
}


auto LogicalDevice::create_texture_image_view() -> void {
    // m_texture_image_view = this->create_image_view(m_texture_image, VK_FORMAT_R8G8B8A8_SRGB);
    m_texture_image_view.init(*this, m_texture_image, VK_FORMAT_R8G8B8A8_SRGB);
}


auto LogicalDevice::create_texture_sampler() -> void {
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
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = m_physical_device->m_properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    result = vkCreateSampler(m_handle, &samplerInfo, nullptr, &m_texture_sampler);
    JF_ASSERT(result == VK_SUCCESS, "");
}

/*---------------------------
    Buffer
---------------------------*/

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



Buffer::Buffer(const LogicalDevice& device, Buffer::TYPE buffer_type, void* data, size_t size) {
    /*VkResult result;*/
    m_device = &device;
    m_size = size;
    m_type = buffer_type;

    bool                  b_with_staging_buffer = false;
    VkBufferUsageFlags    usage = {};
    VkMemoryPropertyFlags properties = {};

    switch (buffer_type) {
        case Buffer::TYPE::VERTEX: {
            JF_ASSERT(m_type == Buffer::TYPE::VERTEX, "");
            b_with_staging_buffer = true;
            usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        } break;
        case Buffer::TYPE::INDEX: {
            JF_ASSERT(m_type == Buffer::TYPE::INDEX, "");
            if (m_type != Buffer::TYPE::INDEX) JF_ASSERT(false, "");
            b_with_staging_buffer = true;
            usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        } break;
        case Buffer::TYPE::UNIFORM: {
            JF_ASSERT(m_type == Buffer::TYPE::UNIFORM, "Expected uniform buffer, got something else");
            b_with_staging_buffer = false;
            usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        } break;
        case Buffer::TYPE::STAGING: {
            JF_ASSERT(m_type == Buffer::TYPE::STAGING, "");
            b_with_staging_buffer = false;
            usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        } break;
        default: JF_ASSERT(false, ""); break;
    }

    if (b_with_staging_buffer == true) {
        Buffer staging_buffer(device, Buffer::TYPE::STAGING, nullptr, size);
        staging_buffer.write(data, 0, size);

        this->create_buffer(size, usage, properties, m_handle, m_memory);
        this->copy_buffer(staging_buffer.m_handle, m_handle, size);
        staging_buffer.deinit();
    } else {
        assert(data == nullptr);
        this->create_buffer(size, usage, properties, m_handle, m_memory);
    }
}


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

auto Buffer::init(const LogicalDevice& device, Buffer::TYPE buffer_type, void* data, size_t size) -> void {
    /*VkResult result;*/
    m_device = &device;
    m_size = size;
    m_type = buffer_type;

    bool                  b_with_staging_buffer = false;
    VkBufferUsageFlags    usage = {};
    VkMemoryPropertyFlags properties = {};

    switch (buffer_type) {
        case Buffer::TYPE::VERTEX: {
            JF_ASSERT(m_type == Buffer::TYPE::VERTEX, "");
            b_with_staging_buffer = true;
            usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        } break;
        case Buffer::TYPE::INDEX: {
            JF_ASSERT(m_type == Buffer::TYPE::INDEX, "");
            if (m_type != Buffer::TYPE::INDEX) JF_ASSERT(false, "");
            b_with_staging_buffer = true;
            usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        } break;
        case Buffer::TYPE::UNIFORM: {
            JF_ASSERT(m_type == Buffer::TYPE::UNIFORM, "Expected uniform buffer, got something else");
            b_with_staging_buffer = false;
            usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        } break;
        case Buffer::TYPE::STAGING: {
            JF_ASSERT(m_type == Buffer::TYPE::STAGING, "");
            b_with_staging_buffer = false;
            usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        } break;
        default: JF_ASSERT(false, ""); break;
    }

    if (b_with_staging_buffer == true) {
        Buffer staging_buffer(device, Buffer::TYPE::STAGING, nullptr, size);
        staging_buffer.write(data, 0, size);

        this->create_buffer(size, usage, properties, m_handle, m_memory);
        this->copy_buffer(staging_buffer.m_handle, m_handle, size);
        staging_buffer.deinit();
    } else {
        assert(data == nullptr);
        this->create_buffer(size, usage, properties, m_handle, m_memory);
    }
}


auto Buffer::deinit() -> void {
    vkDestroyBuffer(m_device->m_handle, m_handle, nullptr);
    vkFreeMemory(m_device->m_handle, m_memory, nullptr);

    Logger::info("Destroyed Buffer {} at {}", fmt::ptr(this), fmt::ptr(m_handle));

    m_handle = VK_NULL_HANDLE;
    m_memory = VK_NULL_HANDLE;
}

auto Buffer::write(const Matrix4x4& m, VkDeviceSize offset) -> void { this->write((void*)&m, offset, sizeof(m)); }
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
    if (size == m_size) return;

    this->deinit();
    this->init(*m_device, m_type, nullptr, size);
}

auto Buffer::create_buffer(
    VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
    VkDeviceMemory& buffer_memory) -> void {
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
    result = vkCreateBuffer(m_device->m_handle, &buffer_info, nullptr, &buffer);
    JF_ASSERT(result == VK_SUCCESS, "");
    m_handle = buffer;

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(m_device->m_handle, buffer, &mem_requirements);

    const VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = m_device->m_physical_device->find_memory_type(mem_requirements.memoryTypeBits, properties),
    };

    result = vkAllocateMemory(m_device->m_handle, &alloc_info, nullptr, &buffer_memory);
    JF_ASSERT(result == VK_SUCCESS, "");

    result = vkBindBufferMemory(m_device->m_handle, buffer, buffer_memory, 0);
    { Logger::info("Created Buffer {} at {}", fmt::ptr(this), fmt::ptr(m_handle)); }
}

auto Buffer::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void {
    const CommandPool& cmd_pool = m_device->m_command_pool;

    CommandBuffer cmd = cmd_pool.allocate_buffer();

    cmd.record([&] {
        const VkBufferCopy copy_region = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };
        vkCmdCopyBuffer(cmd.m_handle, src_buffer, dst_buffer, 1, &copy_region);
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
    const LogicalDevice& device, const VertexData& vertex_data, const VertexFormat /*vertex_format*/,
    bool interleaved) {
    const std::vector<f32> data_ = convert_into_data(vertex_data, interleaved);

    auto data = (void*)data_.data();
    auto size = sizeof(data_[0]) * data_.size();
    m_vertex_buffer = device.create_buffer(Buffer::TYPE::VERTEX, data, size);

    if (vertex_data.m_indices.size() > 0) {
        auto& i_data = vertex_data.m_indices;
        auto  data = (void*)i_data.data();
        auto  size = sizeof(i_data[0]) * i_data.size();
        m_index_buffer = device.create_buffer(Buffer::TYPE::INDEX, data, size);
    }
}

auto GPUMeshData::bind() const -> void {}
auto GPUMeshData::set_layout(const VertexFormat& /*vertex_format*/) -> void {}

/*---------------------------
    Image
---------------------------*/
Image::~Image() {
    if (m_handle != VK_NULL_HANDLE) { this->deinit(); }
}
Image::Image(Image&& other) noexcept
    : m_handle(other.m_handle)
    , m_device(other.m_device)
    , m_memory(other.m_memory)
    , m_source(other.m_source) {
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_memory = VK_NULL_HANDLE;
    other.m_source = SOURCE::REGULAR;
}
auto Image::operator=(Image&& other) -> Image& {
    m_handle = other.m_handle;
    m_device = other.m_device;
    m_memory = other.m_memory;
    m_source = other.m_source;
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_memory = VK_NULL_HANDLE;
    other.m_source = SOURCE::REGULAR;
    return *this;
}

auto Image::init(const LogicalDevice& device, const v2u32& size, VkFormat format, VkImageUsageFlags usage) -> void {
    m_device = &device;
    m_source = SOURCE::REGULAR;

    VkResult                result;
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

    result = vkCreateImage(device.m_handle, &image_info, nullptr, &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
    {
        Logger::info("Created image {} at {}", fmt::ptr(this), fmt::ptr(m_handle));
        Logger::info(
            "-w, h, d: {}, {}, {}", image_info.extent.width, image_info.extent.height, image_info.extent.depth);
        Logger::info("-mip levels: {}", image_info.mipLevels);
        Logger::info("-array layers: {}", image_info.arrayLayers);
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device.m_handle, m_handle, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = device.m_physical_device->find_memory_type(
            mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    result = vkAllocateMemory(device.m_handle, &alloc_info, nullptr, &m_memory);
    JF_ASSERT(result == VK_SUCCESS, "");

    result = vkBindImageMemory(device.m_handle, m_handle, m_memory, 0);
    JF_ASSERT(result == VK_SUCCESS, "");
}
auto Image::init(const LogicalDevice& device, VkImage image) -> void {
    m_device = &device;
    m_handle = image;
    m_source = SOURCE::SWAPCHAIN;
}
auto Image::deinit() -> void {
    switch (m_source) {
        case SOURCE::REGULAR: {
            JF_ASSERT(false, "");
        } break;
        case SOURCE::SWAPCHAIN: {

        } break;
        default: JF_ASSERT(false, "");
    }
}

/*---------------------------
    Image View
---------------------------*/

ImageView::~ImageView() {
    if (m_handle != VK_NULL_HANDLE) { this->deinit(); }
}
ImageView::ImageView(ImageView&& other) noexcept
    : m_handle(other.m_handle)
    , m_device(other.m_device)
    , m_image(other.m_image) {
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_image = nullptr;
}
auto ImageView::operator=(ImageView&& other) -> ImageView& {
    m_handle = other.m_handle;
    m_device = other.m_device;
    m_image = other.m_image;
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_image = nullptr;
    return *this;
}

auto ImageView::init(const LogicalDevice& device, const Image& image, VkFormat format) -> void {
    m_device = &device;
    m_image = &image;
    VkResult              result;
    VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
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

    result = vkCreateImageView(device.m_handle, &create_info, nullptr, &m_handle);
    if (result != VK_SUCCESS) assert(false);
}
auto ImageView::deinit() -> void { vkDestroyImageView(m_device->m_handle, m_handle, nullptr); }

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
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = device.m_physical_device->m_properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    result = vkCreateSampler(device.m_handle, &samplerInfo, nullptr, &m_handle);
    if (result != VK_SUCCESS) assert(false);
}

auto Sampler::deinit() -> void {}

/*---------------------------
        Texture
---------------------------*/

Vulkan_Texture::Vulkan_Texture(const LogicalDevice& device, void* data, v2u32 size, VkFormat format) {
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
    m_image.init(device, size, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    this->transition_layout(m_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    this->copy_buffer_to_image(staging_buffer, m_image, size);
    this->transition_layout(
        m_image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    staging_buffer.deinit();

    m_sampler.init(device);
}
auto Vulkan_Texture::deinit() -> void {}
auto Vulkan_Texture::transition_layout(
    const Image& image, VkFormat /*format*/, VkImageLayout old_layout, VkImageLayout new_layout) -> void {
    const LogicalDevice& d = *m_device;

    auto cb = d.m_command_pool.allocate_buffer();

    cb.record([&] {
        VkImageMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
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

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (
            old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            assert(false);
        }

        vkCmdPipelineBarrier(cb.m_handle, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    });
    // cb[0].record_end();

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cb.m_handle,
    };
    d.m_graphics_queue.submit(submit_info, VK_NULL_HANDLE);
    d.m_graphics_queue.wait_idle();

    d.m_command_pool.free_buffer(cb);
}
auto Vulkan_Texture::copy_buffer_to_image(const Buffer& buffer, const Image& image, v2u32 size) -> void {
    const LogicalDevice& d = *m_device;

    auto cb = d.m_command_pool.allocate_buffer();
    cb.record([&] {
        const VkBufferImageCopy region = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
            .imageOffset = {0, 0, 0},
            .imageExtent = {size.width, size.height, 1},
        };

        vkCmdCopyBufferToImage(
            cb.m_handle, buffer.m_handle, image.m_handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    });
    // cb[0].record_end();

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cb.m_handle,
    };
    // d.m_graphics_queue.submit(submit_info, VK_NULL_HANDLE);
    d.m_graphics_queue.submit(cb, nullptr, nullptr, nullptr);
    d.m_graphics_queue.wait_idle();

    d.m_command_pool.free_buffer(cb);
}
} // namespace vulkan
} // namespace JadeFrame