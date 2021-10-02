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
	auto update() -> void;
	auto add_uniform_buffer(
		const VulkanBuffer& buffer,
		VkDeviceSize offset,
		u32 binding
	) -> void;
public:
	VkDescriptorSet m_handle;
	const VulkanLogicalDevice* m_device = nullptr;
	std::vector<VkDescriptorBufferInfo> m_descriptor_buffer_infos;
};
}