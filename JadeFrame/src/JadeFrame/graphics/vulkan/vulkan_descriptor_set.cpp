#include "pch.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_buffer.h"
#include "JadeFrame/utils/utils.h"

#include <vector>
#include <cassert>

namespace JadeFrame {

class VulkanLogicalDevice;
/*---------------------------
	Descriptor Set
---------------------------*/
auto VulkanDescriptorSet::add_uniform_buffer(const VulkanBuffer& buffer, u32 binding, VkDeviceSize offset, VkDeviceSize range) -> void {
	//NOTE: Vulkan Spec only guarantees 16K addressable space, while on most Desktop platforms it's 64K
	if (buffer.m_size > from_kibibyte(64)) {
		__debugbreak();
	}

	VulkanDescriptor d = {
		.info = {
			.buffer = buffer.m_handle,
			.offset = offset,
			.range = range,
		},
		.binding = binding,
	};

	if (!(d.info.offset < buffer.m_size)) __debugbreak();
	if (d.info.range != VK_WHOLE_SIZE && !(d.info.range > 0)) __debugbreak();
	if (d.info.range != VK_WHOLE_SIZE && !(d.info.range <= buffer.m_size - d.info.offset)) __debugbreak();

	//Find according to binding.
	//TODO: Maybe find a better way
	bool found = false;
	for (u32 i = 0; i < m_descriptors.size(); i++) {
		if(m_descriptors[i].binding == d.binding) {
			found = true;
			m_descriptors[i].info = d.info;
		}
	}
	if (found == false) __debugbreak();

}
auto VulkanDescriptorSet::update() -> void {

	//TODO: Look whether there is a better way to extract that
	std::vector<VkDescriptorBufferInfo> dbi;
	dbi.resize(m_descriptors.size());
	for (u32 i = 0; i < m_descriptors.size(); i++) {
		dbi[i] = m_descriptors[i].info;
	}

	std::vector<VkWriteDescriptorSet> wdss;
	wdss.reserve(m_descriptors.size());


	for (u32 i = 0; i < m_descriptors.size(); i++) {

		const VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_handle,
			.dstBinding = m_descriptors[i].binding,
			.dstArrayElement = 0,
			.descriptorCount = 1/*static_cast<u32>(m_descriptors.size())*/,
			.descriptorType = m_descriptors[i].type,//m_binding_map.at(m_descriptors[i].binding).descriptorType,//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = nullptr,
			.pBufferInfo = dbi.data(),
			.pTexelBufferView = nullptr,
		};
		wdss.push_back(wds);
	}

	vkUpdateDescriptorSets(m_device->m_handle, wdss.size(), wdss.data(), 0, nullptr);
}



/*---------------------------
	Descriptor Set Layout
---------------------------*/

auto VulkanDescriptorSetLayout::add_binding(u32 binding, VkDescriptorType descriptor_type, u32 descriptor_count, VkShaderStageFlags stage_flags, const VkSampler* p_immutable_samplers) -> void {
	if (m_handle != VK_NULL_HANDLE) __debugbreak();
	const VkDescriptorSetLayoutBinding dslb = {
		.binding = binding,
		.descriptorType = descriptor_type,
		.descriptorCount = descriptor_count,
		.stageFlags = stage_flags,
		.pImmutableSamplers = p_immutable_samplers
	};
	if (dslb.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT && !(dslb.descriptorCount % 4 == 0)) __debugbreak();
	m_bindings.push_back(dslb);
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

/*---------------------------
	Descriptor Pool
---------------------------*/
auto VulkanDescriptorPool::add_pool_size(const VkDescriptorPoolSize& pool_size) -> void {
	if (m_handle != VK_NULL_HANDLE) __debugbreak();

	if (!(pool_size.descriptorCount > 0)) __debugbreak();

	m_pool_sizes.push_back(pool_size);
}


auto VulkanDescriptorPool::init(const VulkanLogicalDevice& device, u32 max_sets) -> void {
	m_device = &device;
	VkResult result;
	const VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0 /* | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/ ,
		.maxSets = max_sets,
		.poolSizeCount = static_cast<u32>(m_pool_sizes.size()),
		.pPoolSizes = m_pool_sizes.data(),
	};
	if (!(pool_info.maxSets > 0)) __debugbreak();
	if (!(pool_info.poolSizeCount > 0)) __debugbreak();

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
	result = vkAllocateDescriptorSets(m_device->m_handle, &alloc_info, handles.data());
	if (result != VK_SUCCESS) __debugbreak();

	std::vector<VulkanDescriptorSet> descriptor_sets;
	descriptor_sets.resize(handles.size());
	for (u32 i = 0; i < descriptor_sets.size(); i++) {
		VulkanDescriptorSet& set = descriptor_sets[i];
		set.m_handle = handles[i];
		set.m_device = m_device;

		set.m_descriptors.resize(descriptor_set_layout.m_bindings.size());
		for (u32 j = 0; j < descriptor_set_layout.m_bindings.size(); j++) {
			set.m_descriptors[j].binding = descriptor_set_layout.m_bindings[j].binding;
			//set.m_descriptors[j].info = descriptor_set_layout.m_bindings[j].;
			set.m_descriptors[j].stage_flags = descriptor_set_layout.m_bindings[j].stageFlags;
			set.m_descriptors[j].type = descriptor_set_layout.m_bindings[j].descriptorType;
		}
		//for (const VkDescriptorSetLayoutBinding& dslb : descriptor_set_layout.m_bindings) {
		//	set.m_binding_map.insert({ dslb.binding, dslb });
		//}
	}
	return descriptor_sets;

}

auto VulkanDescriptorPool::allocate_descriptor_set(const VulkanDescriptorSetLayout& descriptor_set_layout) -> VulkanDescriptorSet {
	return this->allocate_descriptor_sets(descriptor_set_layout, 1)[0];
}

auto VulkanDescriptorPool::free_descriptor_sets(const std::vector<VulkanDescriptorSet>& descriptor_sets) -> void {
	//for(u32 i = 0; i < descriptor_sets.size(); i++) {
	//	VkResult result;
	//	result = vkFreeDescriptorSets(m_device->m_handle, m_handle, 1, &descriptor_sets[i].m_handle);
	//	if (result != VK_SUCCESS) __debugbreak();
	//}
	//VkResult result;
	//result = vkResetDescriptorPool(m_device->m_handle, m_handle, 0);
	//if (result != VK_SUCCESS) __debugbreak();
	vkDestroyDescriptorPool(m_device->m_handle, m_handle, nullptr);

}

auto VulkanDescriptorPool::free_descriptor_set(const VulkanDescriptorSet& descriptor_set) -> void {
	VkResult result;
	result = vkFreeDescriptorSets(m_device->m_handle, m_handle, 1, &descriptor_set.m_handle);
	if (result != VK_SUCCESS) __debugbreak();
}

}