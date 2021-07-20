#include "vulkan_descriptor_set.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_descriptor_set_layout.h"
#include "vulkan_buffer.h"

#include <vector>
#include <cassert>

namespace JadeFrame {

class VulkanLogicalDevice;
auto VulkanDescriptorSets::init(const VulkanLogicalDevice& device, u32 image_amount, const VulkanDescriptorSetLayout& descriptor_set_layout, const VulkanDescriptorPool& descriptor_pool) -> void {

	VkResult result;

	std::vector<VkDescriptorSetLayout> layouts(image_amount, device.m_descriptor_set_layout.m_handle);

	const VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = descriptor_pool.m_handle,
		.descriptorSetCount = static_cast<u32>(image_amount),
		.pSetLayouts = layouts.data(),
	};

	m_descriptor_sets.resize(image_amount);
	assert(device.m_physical_device_p->m_properties.limits.maxBoundDescriptorSets > m_descriptor_sets.size());
	//__debugbreak();
	result = vkAllocateDescriptorSets(device.m_handle, &alloc_info, m_descriptor_sets.data());
	if (result != VK_SUCCESS) __debugbreak();

	//for(u32 i = 0; i < image_amount; i++) {
	//	this->update(image_amount, device.m_uniform_buffers);
	//}
	for (u32 i = 0; i < image_amount; i++) {
		const VkDescriptorBufferInfo buffer_info = {
			.buffer = device.m_uniform_buffers[i].m_buffer,
			.offset = 0,
			.range = sizeof(UniformBufferObject),
		};

		const VkWriteDescriptorSet descriptor_write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_descriptor_sets[i],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = nullptr,
			.pBufferInfo = &buffer_info,
			.pTexelBufferView = nullptr,
		};

		vkUpdateDescriptorSets(device.m_handle, 1, &descriptor_write, 0, nullptr);
	}

}

auto VulkanDescriptorSets::update(u32 image_amount, const std::vector<VulkanBuffer>& uniform_buffers) -> void {
	VkDescriptorBufferInfo buffer_info = {};
	buffer_info.buffer = uniform_buffers[image_amount].m_buffer;
	buffer_info.offset = 0;
	buffer_info.range = sizeof(UniformBufferObject);

	VkWriteDescriptorSet descriptor_write = {};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.pNext = nullptr;
	descriptor_write.dstSet = m_descriptor_sets[image_amount];
	descriptor_write.dstBinding = 0;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorCount = 1;
	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_write.pImageInfo = nullptr;
	descriptor_write.pBufferInfo = &buffer_info;
	descriptor_write.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(m_device->m_handle, 1, &descriptor_write, 0, nullptr);
}

}