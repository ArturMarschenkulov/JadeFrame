#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {

class VulkanLogicalDevice;

class VulkanDescriptorPool {
public:
	auto init(const VulkanLogicalDevice& device) -> void;

public:
	const VulkanLogicalDevice* m_device = nullptr;
	VkDescriptorPool m_handle = VK_NULL_HANDLE;
};

}