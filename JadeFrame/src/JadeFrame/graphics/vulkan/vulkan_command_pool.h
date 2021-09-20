#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_command_buffers.h"
#include "JadeFrame/defines.h"

namespace JadeFrame {

class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class VulkanCommandBuffers;

class VulkanCommandPool {
public:
	auto init(const VulkanLogicalDevice& device, const VulkanPhysicalDevice& physical_device) -> void;
	auto deinit() -> void;

	auto allocate_command_buffers(u32 amount = 1) -> VulkanCommandBuffers;
public:
	const VulkanLogicalDevice* m_device = nullptr;
	VkCommandPool m_handle = VK_NULL_HANDLE;
};
}