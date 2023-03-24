#pragma once
#include <vulkan/vulkan.h>

#include "vulkan_shared.h"
#include "../graphics_shared.h"
#include "../mesh.h"

#include "JadeFrame/prelude.h"

#include <vector>

namespace JadeFrame {



// enum class VULKAN_BUFFER_TYPE {
//	UNINIT, // TODO: find ways to remove it
//	VERTEX,
//	INDEX,
//	UNIFORM,
//	STAGING,
// };
namespace vulkan {
class LogicalDevice;
class PhysicalDevice;
class Buffer {
public:
    enum TYPE {
        UNINIT, // TODO: find ways to remove it
        VERTEX,
        INDEX,
        UNIFORM,
        STAGING
    };
    Buffer() = default;
    ~Buffer() = default;
    Buffer(const Buffer&) = delete;
    auto operator=(const Buffer&) -> Buffer& = delete;
    Buffer(Buffer&& other);
    auto operator=(Buffer&& other) -> Buffer&;

    Buffer(const Buffer::TYPE type);


    auto init(const LogicalDevice& device, Buffer::TYPE buffer_type, void* data, size_t size) -> void;
    auto deinit() -> void;
    
    auto send(const Matrix4x4& m, VkDeviceSize offset) -> void;
    auto send(void* data, VkDeviceSize offset, VkDeviceSize size) -> void;
    auto resize(size_t size) -> void;

private:
    auto create_buffer(
        VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
        VkDeviceMemory& buffer_memory) -> void;
    auto copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void;

public:
    /*const*/ Buffer::TYPE m_type = Buffer::TYPE::UNINIT;
    //	VkBufferUsageFlags m_usage = 0;
    VkDeviceSize m_size = 0;

    VkBuffer       m_handle = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;

    const LogicalDevice* m_device = nullptr;
};


class Vulkan_GPUMeshData {
public:
    Vulkan_GPUMeshData() = default;
    ~Vulkan_GPUMeshData() = default;

    Vulkan_GPUMeshData(const Vulkan_GPUMeshData&) = delete;
    auto operator=(const Vulkan_GPUMeshData&) -> Vulkan_GPUMeshData& = delete;

    // Vulkan_GPUMeshData(Vulkan_GPUMeshData&& other);
    auto operator=(Vulkan_GPUMeshData&& other) -> Vulkan_GPUMeshData&;

    Vulkan_GPUMeshData(
        const LogicalDevice& device, const VertexData& vertex_data, const VertexFormat vertex_format,
        bool interleaved = true);

    auto bind() const -> void;
    auto set_layout(const VertexFormat& vertex_format) -> void;

public:
    Buffer       m_vertex_buffer = Buffer::TYPE::VERTEX;
    Buffer       m_index_buffer = Buffer::TYPE::INDEX;
    VertexFormat m_vertex_format;
};

class Image {
public:
    enum class SOURCE {
        REGULAR,
        SWAPCHAIN
    };
    Image() = default;
    ~Image();
    Image(const Image&) = delete;
    auto operator=(const Image&) -> Image& = delete;
    Image(Image&& other) noexcept;
    auto operator=(Image&& other) -> Image&;

    auto init(const LogicalDevice& device, const v2u32& extent, VkFormat format, VkImageUsageFlags usage) -> void;
    auto init(const LogicalDevice& device, VkImage image) -> void;
    auto deinit() -> void;

    VkImage              m_handle = VK_NULL_HANDLE;
    const LogicalDevice* m_device = nullptr;
    VkDeviceMemory       m_memory;
    SOURCE               m_source;
};

class ImageView {
public:
    ImageView() = default;
    ~ImageView();
    ImageView(const ImageView&) = delete;
    auto operator=(const ImageView&) -> ImageView& = delete;
    ImageView(ImageView&& other) noexcept;
    auto operator=(ImageView&& other) -> ImageView&;

    auto init(const LogicalDevice& device, const Image& image, VkFormat format) -> void;
    auto deinit() -> void;

public:
    VkImageView          m_handle = VK_NULL_HANDLE;
    const LogicalDevice* m_device = nullptr;
    const Image*         m_image = nullptr;
};


class Sampler {
public:
    Sampler() = default;
    ~Sampler() = default;
    Sampler(const Sampler&) = delete;
    auto operator=(const Sampler&) -> Sampler& = delete;
    Sampler(Sampler&&) = delete;
    auto operator=(Sampler&&) -> Sampler& = delete;

    auto init(const LogicalDevice& device) -> void;
    auto deinit() -> void;

public:
    VkSampler            m_handle;
    const LogicalDevice* m_device;
};
class Vulkan_Texture {
public:
    Vulkan_Texture() = default;
    ~Vulkan_Texture() = default;
    Vulkan_Texture(const Vulkan_Texture&) = delete;
    auto operator=(const Vulkan_Texture&) -> Vulkan_Texture& = delete;
    Vulkan_Texture(Vulkan_Texture&&) = delete;
    auto operator=(Vulkan_Texture&&) -> Vulkan_Texture& = delete;

    auto init(const LogicalDevice& device, void* data, v2u32 size, VkFormat) -> void;
    auto deinit() -> void;

    auto transition_layout(const Image& image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
        -> void;
    auto copy_buffer_to_image(const Buffer& buffer, const Image& image, v2u32 size) -> void;

public:
    Image                m_image;
    ImageView            m_image_view;
    const LogicalDevice* m_device;
    Sampler              m_sampler;
};
} // namespace vulkan

} // namespace JadeFrame