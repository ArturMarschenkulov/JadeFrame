#pragma once
#include <vulkan/vulkan.h>

#include "vulkan_shared.h"
#include "../graphics_shared.h"
#include "../mesh.h"

#include "JadeFrame/defines.h"

#include <vector>

namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanPhysicalDevice;

//enum class VULKAN_BUFFER_TYPE {
//	UNINIT, // TODO: find ways to remove it
//	VERTEX,
//	INDEX,
//	UNIFORM,
//	STAGING,
//};

class VulkanBuffer {
public:
	enum TYPE {
		UNINIT, // TODO: find ways to remove it
		VERTEX,
		INDEX,
		UNIFORM,
		STAGING
	};
	VulkanBuffer() = default;
	VulkanBuffer(const VulkanBuffer::TYPE type);
	auto init(const VulkanLogicalDevice& device, VulkanBuffer::TYPE buffer_type, void* data, size_t size) -> void;
	auto deinit() -> void;
	auto send(void* data, VkDeviceSize offset, VkDeviceSize size) -> void;
	auto resize(size_t size) -> void;
private:
	auto create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory) -> void;
	auto copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void;

public:
	const VulkanBuffer::TYPE m_type = VulkanBuffer::TYPE::UNINIT;
	//	VkBufferUsageFlags m_usage = 0;
	VkDeviceSize m_size = 0;

	VkBuffer m_handle = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;

	const VulkanLogicalDevice* m_device = nullptr;

};


class Vulkan_GPUMeshData {
public:
	Vulkan_GPUMeshData(const VulkanLogicalDevice& device, const VertexData& vertex_data, const VertexFormat& vertex_format, bool interleaved = true);
	auto bind() const -> void;
	auto set_layout(const VertexFormat& vertex_format) -> void;
public:
	VulkanBuffer m_vertex_buffer = VulkanBuffer::TYPE::VERTEX;
	VulkanBuffer m_index_buffer = VulkanBuffer::TYPE::INDEX;
	VertexFormat m_vertex_format;
};

class VulkanImage {
public:
	enum class SOURCE {
		REGULAR,
		SWAPCHAIN
	};
	auto init(const VulkanLogicalDevice& device, const v2u32& extent, VkFormat format, VkImageUsageFlags usage) -> void;
	auto init(const VulkanLogicalDevice& device, VkImage image) -> void;
	auto deinit() -> void;

	VkImage m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;
	VkDeviceMemory m_memory;
	SOURCE m_source;
};

class VulkanImageView {
public:
	auto init(const VulkanLogicalDevice& device, const VulkanImage& image, VkFormat format) -> void;
	auto deinit() -> void;
public:
	VkImageView m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;
	const VulkanImage* m_image = nullptr;
};

class VulkanSampler {
public:
public:
	VkSampler m_handle;
	const VulkanLogicalDevice* m_device;
};
class Vulkan_Texture {
public:
	auto init(const VulkanLogicalDevice& device, void* data, v2u32 size, VkFormat);
	auto deinit() -> void;

	auto transition_layout(const VulkanImage& image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) -> void;
	auto copy_buffer_to_image(const VulkanBuffer buffer, const VulkanImage image, v2u32 size) -> void;
public:
	const VulkanLogicalDevice* m_device;
};

}