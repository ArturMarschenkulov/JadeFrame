#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/defines.h"
#include <vector>
namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanDescriptorSetLayout {

public:
	auto init(const VulkanLogicalDevice& device) -> void;
	auto deinit() -> void;

	auto add_binding(
		u32 binding, 
		VkDescriptorType descriptor_type, 
		u32 descriptor_count,
		VkShaderStageFlags stage_flags,
		const VkSampler* p_immutable_samplers = nullptr
	) -> void;
public:
	VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;
	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
};

}