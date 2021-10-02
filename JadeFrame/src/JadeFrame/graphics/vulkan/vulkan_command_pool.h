#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/defines.h"

namespace JadeFrame {

class QueueFamilyIndices;
class VulkanLogicalDevice;
class VulkanCommandBuffer;

class VulkanCommandPool {
public:
	auto init(const VulkanLogicalDevice& device, const QueueFamilyIndices& queue_family_indices) -> void;
	auto deinit() -> void;

	auto allocate_command_buffers(u32 amount) const -> std::vector<VulkanCommandBuffer>;
	auto allocate_command_buffer() const -> VulkanCommandBuffer;
	auto free_command_buffers(const std::vector<VulkanCommandBuffer>& command_buffers) const -> void;
public:
	const VulkanLogicalDevice* m_device = nullptr;
	VkCommandPool m_handle = VK_NULL_HANDLE;
};
}