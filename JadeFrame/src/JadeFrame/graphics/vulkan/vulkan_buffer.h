#pragma once
#include <vulkan/vulkan.h>

#include "vulkan_shared.h"

#include "JadeFrame/defines.h"
#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/vec_3.h"

#include <vector>

namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanPhysicalDevice;

enum class VULKAN_BUFFER_TYPE {
	UNINIT, // TODO: find ways to remove it
	VERTEX,
	INDEX,
	UNIFORM,
	STAGING,
};

auto find_memory_type(const VulkanPhysicalDevice& physical_device, u32 type_filter, VkMemoryPropertyFlags properties) -> u32;

class VulkanBuffer {
public:
	VulkanBuffer() = default;
	VulkanBuffer(const VULKAN_BUFFER_TYPE type);
	auto init(const VulkanLogicalDevice& device, VULKAN_BUFFER_TYPE buffer_type, void* data, size_t size) -> void;
	auto deinit() -> void;
	auto map_to_GPU(void* data, VkDeviceSize size) -> void*;
private:
	auto create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory) -> void;
	auto copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void;

public:
	const VULKAN_BUFFER_TYPE m_type = VULKAN_BUFFER_TYPE::UNINIT;
	//	VkBufferUsageFlags m_usage = 0;
	//	VkDeviceSize m_buffer_size = 0;

	VkBuffer m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;

	const VulkanLogicalDevice* m_device = nullptr;

};
}