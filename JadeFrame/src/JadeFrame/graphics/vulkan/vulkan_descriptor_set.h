#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/defines.h"

#include <vector>

namespace JadeFrame {
class VulkanLogicalDevice;
class VulkanDescriptorSetLayout;
class VulkanDescriptorPool;
class VulkanBuffer;

class VulkanDescriptorSet {
public:
	auto update(const VulkanBuffer& uniform_buffer) -> void;
public:
	VkDescriptorSet m_handle;
	const VulkanLogicalDevice* m_device = nullptr;
};
}