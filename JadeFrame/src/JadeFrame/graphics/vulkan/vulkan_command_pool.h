#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {

class VulkanPhysicalDevice;
class VulkanCommandPool {
public:
	auto init(VkDevice device, const VulkanPhysicalDevice& physical_device) -> void;
	auto deinit() -> void;
public:
	VkDevice      m_device = VK_NULL_HANDLE;
	VkCommandPool m_handle = VK_NULL_HANDLE;
};
}