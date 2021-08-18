#include "pch.h"
#include "vulkan_descriptor_set_layout.h"
#include "vulkan_logical_device.h"

namespace JadeFrame {

auto VulkanDescriptorSetLayout::init(const VulkanLogicalDevice& device) -> void {

	m_device = &device;
	VkResult result;

	const VkDescriptorSetLayoutBinding ubo_layout_binding = {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr,
	};
	const VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &ubo_layout_binding,
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
}