#include "vulkan_buffer.h"

#include "vulkan_logical_device.h"

#include <iostream>
#include "JadeFrame/math/mat_4.h"
#include "vulkan_logical_device.cpp"

namespace JadeFrame {

VulkanBuffer::VulkanBuffer(const VULKAN_BUFFER_TYPE type)
	: m_type(type) {

}
auto VulkanBuffer::init(const VulkanLogicalDevice& device, const std::vector<VVertex>& vertices) -> void {
	if (m_type != VULKAN_BUFFER_TYPE::VERTEX) __debugbreak();
	m_device = &device;

	VkResult result;

	VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();


	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	this->create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		staging_buffer,
		staging_buffer_memory
	);

	void* data;
	result = vkMapMemory(m_device->m_handle, staging_buffer_memory, 0, buffer_size, 0, &data);
	{
		memcpy(data, vertices.data(), (size_t)buffer_size);
	}
	vkUnmapMemory(m_device->m_handle, staging_buffer_memory);

	this->create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_buffer,
		m_memory
	);
	this->copy_buffer(staging_buffer, m_buffer, buffer_size);

	vkDestroyBuffer(m_device->m_handle, staging_buffer, nullptr);
	vkFreeMemory(m_device->m_handle, staging_buffer_memory, nullptr);
}
auto VulkanBuffer::init(const VulkanLogicalDevice& device, const std::vector<u16>& indices) -> void {
	if (m_type != VULKAN_BUFFER_TYPE::INDEX) __debugbreak();
	m_device = &device;

	VkResult result;

	VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();


	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	this->create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		staging_buffer,
		staging_buffer_memory
	);

	void* data;
	result = vkMapMemory(m_device->m_handle, staging_buffer_memory, 0, buffer_size, 0, &data);
	{
		memcpy(data, indices.data(), (size_t)buffer_size);
	}
	vkUnmapMemory(m_device->m_handle, staging_buffer_memory);

	this->create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_buffer,
		m_memory
	);
	this->copy_buffer(staging_buffer, m_buffer, buffer_size);

	vkDestroyBuffer(m_device->m_handle, staging_buffer, nullptr);
	vkFreeMemory(m_device->m_handle, staging_buffer_memory, nullptr);
}

auto VulkanBuffer::init(const VulkanLogicalDevice& device) -> void {
	if (m_type != VULKAN_BUFFER_TYPE::UNIFORM) __debugbreak();
	m_device = &device;

	VkDeviceSize buffer_size = sizeof(UniformBufferObject);

	this->create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_buffer,
		m_memory
	);
}

auto VulkanBuffer::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory) -> void {
	VkResult result;

	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.pNext = nullptr;
	buffer_info.flags = 0;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_info.queueFamilyIndexCount = 0;
	buffer_info.pQueueFamilyIndices = nullptr;

	result = vkCreateBuffer(m_device->m_handle, &buffer_info, nullptr, &buffer);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}


	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(m_device->m_handle, buffer, &mem_requirements);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = find_memory_type(*m_device->m_physical_device_p, mem_requirements.memoryTypeBits, properties);

	result = vkAllocateMemory(m_device->m_handle, &alloc_info, nullptr, &buffer_memory);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	result = vkBindBufferMemory(m_device->m_handle, buffer, buffer_memory, 0);
}

auto VulkanBuffer::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void {
	VkResult result;

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = m_device->m_command_pool.m_command_pool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	result = vkAllocateCommandBuffers(m_device->m_handle, &alloc_info, &command_buffer);
	if (result != VK_SUCCESS) {
		__debugbreak();
	}

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	result = vkBeginCommandBuffer(command_buffer, &begin_info);
	if (result != VK_SUCCESS) {
		__debugbreak();
	}
	{
		VkBufferCopy copy_region = {};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size = size;
		vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
	}
	result = vkEndCommandBuffer(command_buffer);
	if (result != VK_SUCCESS) {
		__debugbreak();
	}

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	vkQueueSubmit(m_device->m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_device->m_graphics_queue);

	vkFreeCommandBuffers(m_device->m_handle, m_device->m_command_pool.m_command_pool, 1, &command_buffer);
}
}