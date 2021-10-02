#include "pch.h"
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

}