#include "pch.h"
#include "vulkan_buffer.h"

#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_context.h"

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/utils/assert.h"
#include "JadeFrame/utils/utils.h"

#define VMA_IMPLEMENTATION
#include "VulkanMemoryAllocator/include/vk_mem_alloc.h"

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

            VkDeviceMemory memory = VK_NULL_HANDLE;
            vkAllocateMemory(
                m_device->m_handle, &alloc_info, Instance::allocator(), &memory
            );
            m_blocks.push_back(memory);
        }
    }

    const LogicalDevice*        m_device = nullptr;
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
        default: JF_ASSERT(false, ""); return "";
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
#if JF_USE_VMA
static auto get_vma_usage(const Buffer::TYPE type) -> VmaMemoryUsage {
    VmaMemoryUsage result = {};
    switch (type) {
        case Buffer::TYPE::VERTEX:
        case Buffer::TYPE::INDEX: result = VMA_MEMORY_USAGE_GPU_ONLY; break;
        case Buffer::TYPE::UNIFORM:
        case Buffer::TYPE::STAGING: result = VMA_MEMORY_USAGE_CPU_TO_GPU; break;
        case Buffer::TYPE::UNINIT: result = VMA_MEMORY_USAGE_UNKNOWN; break;
        default: JF_ASSERT(false, ""); break;
    }
    return result;
}
#endif

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

Buffer::Buffer(Buffer&& other) noexcept
    : m_type(other.m_type)
    , m_size(other.m_size)
    , m_handle(other.m_handle)
#if JF_USE_VMA
    , m_allocation(other.m_allocation)
#else
    , m_memory(other.m_memory)
#endif
    , m_device(other.m_device) {

    other.m_type = TYPE::UNINIT;
    other.m_size = 0;
    other.m_handle = VK_NULL_HANDLE;
#if JF_USE_VMA
    other.m_allocation = VK_NULL_HANDLE;
#else
    other.m_memory = VK_NULL_HANDLE;
#endif
    other.m_device = nullptr;
}

auto Buffer::operator=(Buffer&& other) noexcept -> Buffer& {
    m_type = other.m_type;
    m_size = other.m_size;
    m_handle = other.m_handle;
#if JF_USE_VMA
    m_allocation = other.m_allocation;
#else
    m_memory = other.m_memory;
#endif
    m_device = other.m_device;

    other.m_type = TYPE::UNINIT;
    other.m_size = 0;
    other.m_handle = VK_NULL_HANDLE;
#if JF_USE_VMA
    other.m_allocation = VK_NULL_HANDLE;
#else
    other.m_memory = VK_NULL_HANDLE;
#endif
    other.m_device = nullptr;
    return *this;
}

#define JF_USE_MANAGED_STAGING_BUFFER 1
#if JF_USE_MANAGED_STAGING_BUFFER
static Buffer* g_staging_buffer = nullptr;
#endif

static auto transfer_through_staging_buffer(
    const LogicalDevice& device,
    void*                data,
    size_t               size,
    Buffer*              buffer
) -> void {
#if JF_USE_MANAGED_STAGING_BUFFER

    if (g_staging_buffer == nullptr) {
        g_staging_buffer = device.create_buffer(Buffer::TYPE::STAGING, nullptr, size);
    }

    if (g_staging_buffer->m_size != size) { g_staging_buffer->resize(size); }

    g_staging_buffer->write(data, size, 0);
    device.m_command_pool.copy_buffer(*g_staging_buffer, *buffer, size);
#else
    Buffer* sb = device.create_buffer(Buffer::TYPE::STAGING, nullptr, size);
    sb->write(data, size, 0);
    device.m_command_pool.copy_buffer(*sb, *buffer, size);
    device.destroy_buffer(sb);
#endif
}

Buffer::Buffer(
    const LogicalDevice& device,
    Buffer::TYPE         buffer_type,
    void*                data,
    size_t               size
)
    : m_type(buffer_type)
    , m_size(size)
    , m_device(&device) {

    bool               b_with_staging_buffer = does_use_staging_buffer(buffer_type);
    VkBufferUsageFlags usage = get_usage(buffer_type);
#if JF_USE_VMA
    VmaMemoryUsage vma_usage = get_vma_usage(buffer_type);
    this->create_buffer(size, usage, vma_usage, m_handle);
#else
    VkMemoryPropertyFlags properties = get_properties(buffer_type);
    this->create_buffer(size, usage, properties, m_memory, m_handle);
#endif

    if (b_with_staging_buffer) {
        transfer_through_staging_buffer(device, data, size, this);
    } else {
        assert(data == nullptr && "If staging buffer is not used, it cannot have data");
    }
}

Buffer::~Buffer() {
    if (m_handle != VK_NULL_HANDLE) {

#if JF_USE_VMA
        vmaDestroyBuffer(m_device->m_vma_allocator, m_handle, m_allocation);
        m_allocation = VK_NULL_HANDLE;
#else
        vkDestroyBuffer(m_device->m_handle, m_handle, Instance::allocator());
        vkFreeMemory(m_device->m_handle, m_memory, Instance::allocator());
        m_memory = VK_NULL_HANDLE;
#endif
        m_handle = VK_NULL_HANDLE;
        Logger::trace("Destroyed Buffer {} at {}", fmt::ptr(this), fmt::ptr(m_handle));
    }
}

auto Buffer::write(const void* data, VkDeviceSize size, VkDeviceSize offset) const
    -> void {

    void* mapped_data;
#if JF_USE_VMA
    VkResult result = vmaMapMemory(m_device->m_vma_allocator, m_allocation, &mapped_data);
    JF_ASSERT(result == VK_SUCCESS, "");
    u8* data_ptr = reinterpret_cast<u8*>(mapped_data) + offset;
    memcpy(data_ptr, data, static_cast<size_t>(size));
    vmaUnmapMemory(m_device->m_vma_allocator, m_allocation);
#else
    VkResult result =
        vkMapMemory(m_device->m_handle, m_memory, offset, size, 0, &mapped_data);
    JF_ASSERT(result == VK_SUCCESS, "");
    memcpy(mapped_data, data, static_cast<size_t>(size));
    vkUnmapMemory(m_device->m_handle, m_memory);
#endif
}

auto Buffer::resize(size_t size) -> void {
    assert(m_type == TYPE::UNIFORM);
    if (size == m_size) { return; }

    this->~Buffer();
    *this = Buffer(*m_device, m_type, nullptr, size);
}

auto Buffer::create_buffer(
    VkDeviceSize       size,
    VkBufferUsageFlags usage,
#if JF_USE_VMA
    VmaMemoryUsage vma_usage,
#else
    VkMemoryPropertyFlags properties,
    VkDeviceMemory&       buffer_memory,

#endif
    VkBuffer& buffer
) -> void {

    const VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };
#if JF_USE_VMA
    VmaAllocationCreateInfo vma_info = {};
    vma_info.usage = vma_usage;
    VkResult result = vmaCreateBuffer(
        m_device->m_vma_allocator, &info, &vma_info, &buffer, &m_allocation, nullptr
    );
#else
    VkResult result =
        vkCreateBuffer(m_device->m_handle, &buffer_info, Instance::allocator(), &buffer);
#endif
    JF_ASSERT(result == VK_SUCCESS, "");
    m_handle = buffer;

    // g_memory.push_buffer(m_device, buffer);

#if JF_USE_VMA
#else

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
#endif
}

GPUMeshData::GPUMeshData(
    const LogicalDevice& device,
    const VertexData&    vertex_data,
    bool                 interleaved
) {

    const std::vector<f32> flat_data = convert_into_data(vertex_data, interleaved);

    void*  data = (void*)flat_data.data();
    size_t size = sizeof(flat_data[0]) * flat_data.size();
    m_vertex_buffer = device.create_buffer(Buffer::TYPE::VERTEX, data, size);

    if (!vertex_data.m_indices.empty()) {
        const auto& indices = vertex_data.m_indices;

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
    if (m_handle == VK_NULL_HANDLE) { return; }
    switch (m_source) {
        case SOURCE::REGULAR: {
            JF_ASSERT(false, "");
        } break;
        case SOURCE::SWAPCHAIN: {

        } break;
        default: JF_ASSERT(false, "");
    }
}

Image::Image(
    const LogicalDevice& device,
    const v2u32&         size,
    VkFormat             format,
    VkImageUsageFlags    usage
)
    : m_device(&device)
    , m_source(SOURCE::REGULAR)
    , m_size(size) {

    VkImageFormatProperties props;

    VkResult result = vkGetPhysicalDeviceImageFormatProperties(
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

Image::Image(const LogicalDevice& device, VkImage image)
    : m_handle(image)
    , m_device(&device)
    , m_source(SOURCE::SWAPCHAIN) {}

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

ImageView::ImageView(const LogicalDevice& device, const Image& image, VkFormat format)
    : m_device(&device)
    , m_image(&image) {

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
        .subresourceRange =
            {
                         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                         .baseMipLevel = 0,
                         .levelCount = 1,
                         .baseArrayLayer = 0,
                         .layerCount = 1,
                         }
    };

    VkResult result = vkCreateImageView(
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
    VkResult result =
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
)
    : m_device(&device) {

    u32 comp_count = 0;
    if (format == VK_FORMAT_R8G8B8_SRGB) {
        comp_count = 3;
    } else if (format == VK_FORMAT_R8G8B8A8_SRGB) {
        comp_count = 4;
    } else {
        assert(false);
    }

    // Image image;
    m_image = Image(
        device, size, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    );
    m_image_view = ImageView(device, m_image, format);

    m_device->m_command_pool.transition_layout(
        m_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    VkDeviceSize image_size = size.width * size.height * comp_count;
    Buffer       staging_buffer(device, Buffer::TYPE::STAGING, nullptr, image_size);
    staging_buffer.write(data, image_size, 0);
    m_device->m_command_pool.copy_buffer_to_image(staging_buffer, m_image, size);

    m_device->m_command_pool.transition_layout(
        m_image,
        format,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    m_sampler.init(device);
}

auto Vulkan_Texture::deinit() -> void {}

} // namespace vulkan
} // namespace JadeFrame