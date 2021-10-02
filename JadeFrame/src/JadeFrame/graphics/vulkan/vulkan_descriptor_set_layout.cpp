#include "pch.h"
#include "vulkan_descriptor_set_layout.h"
#include "vulkan_logical_device.h"

namespace JadeFrame {

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
}