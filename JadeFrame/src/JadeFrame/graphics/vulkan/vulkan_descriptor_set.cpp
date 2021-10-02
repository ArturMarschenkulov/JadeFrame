#include "pch.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_buffer.h"

#include <vector>
#include <cassert>

namespace JadeFrame {

class VulkanLogicalDevice;

auto VulkanDescriptorSet::update() -> void {
	//VkDescriptorBufferInfo buffer_info = {
	//	.buffer = uniform_buffer.m_buffer,
	//	.offset = 0,
	//	.range = sizeof(UniformBufferObject),
	//};

	std::vector< VkWriteDescriptorSet> write_descriptor_sets;
	for (u32 i = 0; i < m_descriptor_buffer_infos.size(); i++) {
		const VkWriteDescriptorSet write_descriptor_set = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_handle,
			.dstBinding = 0,
			.dstArrayElement = 0, //TODO: Make more dynamic
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = nullptr,
			.pBufferInfo = &m_descriptor_buffer_infos[i],
			.pTexelBufferView = nullptr,
		};
		write_descriptor_sets.push_back(write_descriptor_set);
	}

	vkUpdateDescriptorSets(m_device->m_handle, 1, write_descriptor_sets.data(), 0, nullptr);
}

auto VulkanDescriptorSet::add_uniform_buffer(const VulkanBuffer& buffer, VkDeviceSize offset, u32 binding) -> void {
	//if (m_handle != VK_NULL_HANDLE) __debugbreak();

	VkDescriptorBufferInfo dbi = {};
	dbi.buffer = buffer.m_handle;
	dbi.offset = offset;
	dbi.range = buffer.m_size;

	m_descriptor_buffer_infos.push_back(dbi);

}

auto VulkanDescriptorSetLayout::init(const VulkanLogicalDevice& device) -> void {

	m_device = &device;
	VkResult result;

	const VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = static_cast<u32>(m_bindings.size()),
		.pBindings = m_bindings.data(),
	};

	result = vkCreateDescriptorSetLayout(device.m_handle, &layout_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) {
		__debugbreak();
		//throw std::runtime_error("failed to create descriptor set layout!");
	}
}
auto VulkanDescriptorSetLayout::deinit() -> void {
	vkDestroyDescriptorSetLayout(m_device->m_handle, m_handle, nullptr);
}

auto VulkanDescriptorSetLayout::add_binding(u32 binding, VkDescriptorType descriptor_type, u32 descriptor_count, VkShaderStageFlags stage_flags, const VkSampler* p_immutable_samplers) -> void {
	if (m_handle != VK_NULL_HANDLE) __debugbreak();
	const VkDescriptorSetLayoutBinding dslb = {
		.binding = binding,
		.descriptorType = descriptor_type,
		.descriptorCount = descriptor_count,
		.stageFlags = stage_flags,
		.pImmutableSamplers = p_immutable_samplers
	};
	m_bindings.push_back(dslb);
}

auto VulkanDescriptorPool::init(const VulkanLogicalDevice& device, u32 amount) -> void {
	m_device = &device;
	VkResult result;
	const VkDescriptorPoolSize pool_size = {
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = amount,
	};

	const VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = {},
		.flags = 0,
		.maxSets = amount,
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size,
	};

	result = vkCreateDescriptorPool(device.m_handle, &pool_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();
}

auto VulkanDescriptorPool::deinit() -> void {
	vkDestroyDescriptorPool(m_device->m_handle, m_handle, nullptr);
}



auto VulkanDescriptorPool::allocate_descriptor_sets(const VulkanDescriptorSetLayout& descriptor_set_layout, u32 amount) -> std::vector<VulkanDescriptorSet> {
	VkResult result;
	//m_device = &device;
	std::vector<VkDescriptorSetLayout> layouts(amount, descriptor_set_layout.m_handle);

	const VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = m_handle,
		.descriptorSetCount = static_cast<u32>(amount),
		.pSetLayouts = layouts.data(),
	};
	std::vector<VkDescriptorSet> handles(amount);
	//assert(m_device->m_physical_device_p->m_properties.limits.maxBoundDescriptorSets > handles.size());
	result = vkAllocateDescriptorSets(m_device->m_handle, &alloc_info, handles.data());
	if (result != VK_SUCCESS) __debugbreak();

	std::vector<VulkanDescriptorSet> descriptor_sets(handles.size());
	for (u32 i = 0; i < descriptor_sets.size(); i++) {
		descriptor_sets[i].m_handle = handles[i];
		descriptor_sets[i].m_device = m_device;
	}
	return descriptor_sets;

}

}