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
	VkDescriptorBufferInfo info;
	VkDescriptorType type;
	VkShaderStageFlags stage_flags;
	u32 binding;
};

class VulkanDescriptorSet {
public:
	auto update() -> void;
	auto add_uniform_buffer(
		const VulkanBuffer& buffer,
		u32 binding,
		VkDeviceSize offset,
		VkDeviceSize range
	) -> void;
public:
	VkDescriptorSet m_handle;
	const VulkanLogicalDevice* m_device = nullptr;
	
	//Descriptor data. The same index means the same descriptor
	std::vector<VkDescriptorBufferInfo> m_infos;
	std::vector<VkDescriptorType> m_types;
	std::vector<VkShaderStageFlags> m_stage_flags;
	std::vector<u32> m_bindings;
	std::vector<VkDescriptorSetLayoutBinding> m_layout_bindings;
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