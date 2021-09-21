#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/defines.h"

#include <vector>

namespace JadeFrame {
class VulkanLogicalDevice;
class VulkanDescriptorSetLayout;
class VulkanDescriptorPool;
class VulkanBuffer;
class VulkanDescriptorSets {
public:
	auto init(
		const VulkanLogicalDevice& device,
		u32 image_amount,
		const VulkanDescriptorSetLayout& descriptor_set_layout,
		const VulkanDescriptorPool& descriptor_pool
	) -> void;
	auto update(const std::vector<VulkanBuffer>& uniform_buffers) -> void;

	const VulkanLogicalDevice* m_device = nullptr;
	std::vector<VkDescriptorSet> m_descriptor_sets;
};

}