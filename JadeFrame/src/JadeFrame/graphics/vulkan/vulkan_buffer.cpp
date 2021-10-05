#include "pch.h"
#include "vulkan_buffer.h"

#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"

#include "JadeFrame/math/mat_4.h"

namespace JadeFrame {



VulkanBuffer::VulkanBuffer(const VulkanBuffer::TYPE type)
	: m_type(type) {
}

auto VulkanBuffer::init(const VulkanLogicalDevice& device, VulkanBuffer::TYPE buffer_type, void* data, size_t size) -> void {
	VkResult result;
	m_device = &device;
	m_size = size;

	bool b_with_staging_buffer;
	VkBufferUsageFlags usage;
	VkMemoryPropertyFlags properties;

	switch (buffer_type) {
		case VulkanBuffer::TYPE::VERTEX:
		{
			if (m_type != VulkanBuffer::TYPE::VERTEX) __debugbreak();
			b_with_staging_buffer = true;
			usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}break;
		case  VulkanBuffer::TYPE::INDEX:
		{
			if (m_type != VulkanBuffer::TYPE::INDEX) __debugbreak();
			b_with_staging_buffer = true;
			usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}break;
		case  VulkanBuffer::TYPE::UNIFORM:
		{
			if (m_type != VulkanBuffer::TYPE::UNIFORM) __debugbreak();
			b_with_staging_buffer = false;
			usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}break;
		case  VulkanBuffer::TYPE::STAGING:
		{
			if (m_type != VulkanBuffer::TYPE::STAGING) __debugbreak();
			b_with_staging_buffer = false;
			usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}break;
		default: __debugbreak(); break;
	}

	if (b_with_staging_buffer == true) {
		VulkanBuffer staging_buffer = { VulkanBuffer::TYPE::STAGING };
		staging_buffer.init(device, VulkanBuffer::TYPE::STAGING, nullptr, size);


		void* mapped_data = staging_buffer.map_to_GPU(data, size);

		this->create_buffer(
			size,
			usage,
			properties,
			m_handle,
			m_memory
		);
		this->copy_buffer(staging_buffer.m_handle, m_handle, size);
		staging_buffer.deinit();
	} else {
		assert(data == nullptr);
		this->create_buffer(
			size,
			usage,
			properties,
			m_handle,
			m_memory
		);
	}
}


auto VulkanBuffer::deinit() -> void {
	vkDestroyBuffer(m_device->m_handle, m_handle, nullptr);
	vkFreeMemory(m_device->m_handle, m_memory, nullptr);

	m_handle = VK_NULL_HANDLE;
	m_memory = VK_NULL_HANDLE;
}

auto VulkanBuffer::map_to_GPU(void* data, VkDeviceSize size) -> void* {
	VkResult result;

	void* mapped_data;
	result = vkMapMemory(m_device->m_handle, m_memory, 0, size, 0, &mapped_data);
	if (result != VK_SUCCESS) __debugbreak();
	memcpy(mapped_data, data, static_cast<size_t>(size));
	vkUnmapMemory(m_device->m_handle, m_memory);

	return mapped_data;
}

auto VulkanBuffer::resize(size_t size) -> void {
	if (size == m_size) return;

	this->deinit();
	this->init(*m_device, m_type, nullptr, size);
}

auto VulkanBuffer::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory) -> void {
	VkResult result;

	const VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
	};
	result = vkCreateBuffer(m_device->m_handle, &buffer_info, nullptr, &buffer);
	if (result != VK_SUCCESS) __debugbreak();

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(m_device->m_handle, buffer, &mem_requirements);

	const VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = m_device->m_physical_device_p->find_memory_type(mem_requirements.memoryTypeBits, properties),
	};

	result = vkAllocateMemory(m_device->m_handle, &alloc_info, nullptr, &buffer_memory);
	if (result != VK_SUCCESS) __debugbreak();

	result = vkBindBufferMemory(m_device->m_handle, buffer, buffer_memory, 0);
}

auto VulkanBuffer::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void {
	VkResult result;


	const VulkanCommandPool& cp = m_device->m_command_pool;
	std::vector<VulkanCommandBuffer> command_buffer = cp.allocate_command_buffers(1);

	command_buffer[0].record_begin();
	{
		const VkBufferCopy copy_region = {
			.srcOffset = 0,
			.dstOffset = 0,
			.size = size,
		};
		vkCmdCopyBuffer(command_buffer[0].m_handle, src_buffer, dst_buffer, 1, &copy_region);
	}
	command_buffer[0].record_end();

	const VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = {},
		.waitSemaphoreCount = {},
		.pWaitSemaphores = {},
		.pWaitDstStageMask = {},
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer[0].m_handle,
		.signalSemaphoreCount = {},
		.pSignalSemaphores = {},
	};

	result = vkQueueSubmit(m_device->m_graphics_queue.m_handle, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) __debugbreak();
	result = vkQueueWaitIdle(m_device->m_graphics_queue.m_handle);
	if (result != VK_SUCCESS) __debugbreak();

	cp.free_command_buffers(command_buffer);

}
Vulkan_GPUMeshData::Vulkan_GPUMeshData(const VulkanLogicalDevice& device, const VertexData& vertex_data, const VertexFormat& vertex_format, bool interleaved) {
	const std::vector<f32> data = convert_into_data(vertex_data, interleaved);

	m_vertex_buffer.init(device, VulkanBuffer::TYPE::VERTEX, (void*)data.data(), sizeof(data[0]) * data.size());
	if (vertex_data.m_indices.size() > 0) {
		m_index_buffer.init(device, VulkanBuffer::TYPE::INDEX, (void*)vertex_data.m_indices.data(), sizeof(vertex_data.m_indices[0]) * vertex_data.m_indices.size());
	}

}
auto Vulkan_GPUMeshData::bind() const -> void {
}
auto Vulkan_GPUMeshData::set_layout(const VertexFormat& vertex_format) -> void {
}
}