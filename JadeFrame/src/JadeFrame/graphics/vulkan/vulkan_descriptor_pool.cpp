#include "pch.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_swapchain.h"

#include "JadeFrame/defines.h"

namespace JadeFrame {

auto VulkanDescriptorPool::init(const VulkanLogicalDevice& device, const VulkanSwapchain& swapchain) -> void {
	m_device = &device;
	VkResult result;
	const VkDescriptorPoolSize pool_size = {
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = static_cast<u32>(swapchain.m_images.size()),
	};

	const VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.maxSets = static_cast<u32>(swapchain.m_images.size()),
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size,
	};

	result = vkCreateDescriptorPool(device.m_handle, &pool_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();
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
	assert(m_device->m_physical_device_p->m_properties.limits.maxBoundDescriptorSets > handles.size());
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