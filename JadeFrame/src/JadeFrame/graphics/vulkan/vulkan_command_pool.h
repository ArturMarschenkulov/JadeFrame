#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_command_buffers.h"
#include "JadeFrame/defines.h"

namespace JadeFrame {

class QueueFamilyIndices;
class VulkanLogicalDevice;
class VulkanCommandBuffers;

class VulkanCommandPool {
public:
	auto init(const VulkanLogicalDevice& device, const QueueFamilyIndices& queue_family_indices) -> void;
	auto deinit() -> void;

	auto allocate_command_buffers(u32 amount = 1) -> VulkanCommandBuffers;
public:
	const VulkanLogicalDevice* m_device = nullptr;
	VkCommandPool m_handle = VK_NULL_HANDLE;
};
}