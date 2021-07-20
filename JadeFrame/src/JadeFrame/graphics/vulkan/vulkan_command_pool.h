#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {

class VulkanPhysicalDevice;
class VulkanLogicalDevice;
class VulkanCommandPool {
public:
	auto init(const VulkanLogicalDevice& device, const VulkanPhysicalDevice& physical_device) -> void;
	auto deinit() -> void;
public:
	const VulkanLogicalDevice* m_device = nullptr;
	VkCommandPool m_handle = VK_NULL_HANDLE;
};
}