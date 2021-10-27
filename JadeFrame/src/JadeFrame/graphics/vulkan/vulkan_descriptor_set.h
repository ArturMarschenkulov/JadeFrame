#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/defines.h"

#include <vector>

namespace JadeFrame {
class VulkanLogicalDevice;
class VulkanDescriptorSetLayout;
class VulkanDescriptorPool;
class VulkanBuffer;



struct VulkanDescriptor {
	union {
		VkDescriptorBufferInfo bufer_info;
		VkDescriptorImageInfo image_info;
	};
	VkDescriptorType type;
	VkShaderStageFlags stage_flags;
	u32 binding;
};
inline auto is_image(VulkanDescriptor d) -> bool {
	switch (d.type) {
		case VK_DESCRIPTOR_TYPE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return true; break;
			//case VK_DESCRIPTOR_TYPE_SAMPLER:
		default: return false;
	}
}

class VulkanDescriptorSet {
public:
	auto update() -> void;
	auto add_uniform_buffer(
		const VulkanBuffer& buffer,
		u32 binding,
		VkDeviceSize offset,
		VkDeviceSize range
	) -> void;
	auto readd_uniform_buffer(
		u32 binding,
		const VulkanBuffer& buffer
	) -> void;
public:
	VkDescriptorSet m_handle;
	const VulkanLogicalDevice* m_device = nullptr;
	const VulkanDescriptorSetLayout* m_layout = nullptr;

	std::vector<VulkanDescriptor> m_descriptors;

	//su32 m_dynamic_count;
};

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

	u32 m_dynamic_count = 0;
};

class VulkanDescriptorPool {
public:
	auto init(const VulkanLogicalDevice& device, u32 max_sets) -> void;
	auto deinit() -> void;

	auto add_pool_size(const VkDescriptorPoolSize& pool_size) -> void;

	auto allocate_descriptor_sets(const VulkanDescriptorSetLayout& descriptor_set_layout, u32 image_amount)->std::vector<VulkanDescriptorSet>;
	auto allocate_descriptor_set(const VulkanDescriptorSetLayout& descriptor_set_layout)->VulkanDescriptorSet;
	auto free_descriptor_sets(const std::vector<VulkanDescriptorSet>& descriptor_sets) -> void;
	auto free_descriptor_set(const VulkanDescriptorSet& descriptor_sets) -> void;

public:
	const VulkanLogicalDevice* m_device = nullptr;
	VkDescriptorPool m_handle = VK_NULL_HANDLE;
	std::vector<VkDescriptorPoolSize> m_pool_sizes;
};
}