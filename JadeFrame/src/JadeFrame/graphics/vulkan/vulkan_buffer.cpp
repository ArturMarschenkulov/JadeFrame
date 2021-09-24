#include "pch.h"
#include "vulkan_buffer.h"

#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"

#include "JadeFrame/math/mat_4.h"

namespace JadeFrame {



VulkanBuffer::VulkanBuffer(const VULKAN_BUFFER_TYPE type)
	: m_type(type) {
}

auto VulkanBuffer::init(const VulkanLogicalDevice& device, VULKAN_BUFFER_TYPE buffer_type, void* data, size_t size) -> void {
	VkResult result;
	m_device = &device;
#if 0
	switch (buffer_type) {
		//with staging buffer
		case VULKAN_BUFFER_TYPE::VERTEX:
		case VULKAN_BUFFER_TYPE::INDEX:
		{

			VulkanBuffer staging_buffer = { VULKAN_BUFFER_TYPE::STAGING };
			staging_buffer.init(device, VULKAN_BUFFER_TYPE::STAGING, nullptr, size);

			void* mapped_data = staging_buffer.map_to_GPU(data, size);

			VkBufferUsageFlags usage;
			VkMemoryPropertyFlags properties;

			switch (buffer_type) {
				case VULKAN_BUFFER_TYPE::VERTEX:
				{
					if (m_type != VULKAN_BUFFER_TYPE::VERTEX) __debugbreak();

					usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
					properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				}break;
				case VULKAN_BUFFER_TYPE::INDEX:
				{
					if (m_type != VULKAN_BUFFER_TYPE::INDEX) __debugbreak();

					usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
					properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				}break;
				default: __debugbreak();
			}

			this->create_buffer(
				size,
				usage,
				properties,
				m_buffer,
				m_memory
			);
			this->copy_buffer(staging_buffer.m_buffer, m_buffer, size);
			staging_buffer.deinit();
		} break;
		// without staging buffer
		case VULKAN_BUFFER_TYPE::UNIFORM:
		case VULKAN_BUFFER_TYPE::STAGING:
		{

			VkBufferUsageFlags usage;
			VkMemoryPropertyFlags properties;

			switch (buffer_type) {
				case VULKAN_BUFFER_TYPE::UNIFORM:
				{
					if (m_type != VULKAN_BUFFER_TYPE::UNIFORM) __debugbreak();
					usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
					properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				}break;
				case VULKAN_BUFFER_TYPE::STAGING:
				{
					if (m_type != VULKAN_BUFFER_TYPE::STAGING) __debugbreak();
					usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
					properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				}break;
				default: __debugbreak();
			}

			this->create_buffer(
				size,
				usage,
				properties,
				m_buffer,
				m_memory
			);
		} break;
	}
#elif 1
	bool b_with_staging_buffer;
	VkBufferUsageFlags usage;
	VkMemoryPropertyFlags properties;

	switch (buffer_type) {
		case VULKAN_BUFFER_TYPE::VERTEX:
		{
			if (m_type != VULKAN_BUFFER_TYPE::VERTEX) __debugbreak();
			b_with_staging_buffer = true;
			usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}break;
		case VULKAN_BUFFER_TYPE::INDEX:
		{
			if (m_type != VULKAN_BUFFER_TYPE::INDEX) __debugbreak();
			b_with_staging_buffer = true;
			usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}break;
		case VULKAN_BUFFER_TYPE::UNIFORM:
		{
			if (m_type != VULKAN_BUFFER_TYPE::UNIFORM) __debugbreak();
			b_with_staging_buffer = false;
			usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}break;
		case VULKAN_BUFFER_TYPE::STAGING:
		{
			if (m_type != VULKAN_BUFFER_TYPE::STAGING) __debugbreak();
			b_with_staging_buffer = false;
			usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}break;
		default: __debugbreak(); break;
	}

	if (b_with_staging_buffer) {
		VulkanBuffer staging_buffer = { VULKAN_BUFFER_TYPE::STAGING };
		staging_buffer.init(device, VULKAN_BUFFER_TYPE::STAGING, nullptr, size);


		void* mapped_data = staging_buffer.map_to_GPU(data, size);

		this->create_buffer(
			size,
			usage,
			properties,
			m_buffer,
			m_memory
		);
		this->copy_buffer(staging_buffer.m_buffer, m_buffer, size);
		staging_buffer.deinit();
	} else {
		this->create_buffer(
			size,
			usage,
			properties,
			m_buffer,
			m_memory
		);
	}
#else
	switch (buffer_type) {
		case VULKAN_BUFFER_TYPE::VERTEX:
		{
			if (m_type != VULKAN_BUFFER_TYPE::VERTEX) __debugbreak();

			VulkanBuffer staging_buffer = { VULKAN_BUFFER_TYPE::STAGING };
			staging_buffer.init(device, VULKAN_BUFFER_TYPE::STAGING, nullptr, size);

			void* mapped_data = staging_buffer.map_to_GPU(data, size);


			VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			this->create_buffer(
				size,
				usage,
				properties,
				m_buffer,
				m_memory
			);
			this->copy_buffer(staging_buffer.m_buffer, m_buffer, size);
			staging_buffer.deinit();

		}break;
		case VULKAN_BUFFER_TYPE::INDEX:
		{
			if (m_type != VULKAN_BUFFER_TYPE::INDEX) __debugbreak();


			VulkanBuffer staging_buffer = { VULKAN_BUFFER_TYPE::STAGING };
			staging_buffer.init(device, VULKAN_BUFFER_TYPE::STAGING, nullptr, size);

			void* mapped_data = staging_buffer.map_to_GPU(data, size);

			VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			this->create_buffer(
				size,
				usage,
				properties,
				m_buffer,
				m_memory
			);
			this->copy_buffer(staging_buffer.m_buffer, m_buffer, size);
			staging_buffer.deinit();
		}break;
		case VULKAN_BUFFER_TYPE::UNIFORM:
		{
			if (m_type != VULKAN_BUFFER_TYPE::UNIFORM) __debugbreak();

			VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			this->create_buffer(
				size,
				usage,
				properties,
				m_buffer,
				m_memory
			);

		}break;
		case VULKAN_BUFFER_TYPE::STAGING:
		{
			if (m_type != VULKAN_BUFFER_TYPE::STAGING) __debugbreak();
			b_with_staging_buffer = false;

			VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			this->create_buffer(
				size,
				usage,
				properties,
				m_buffer,
				m_memory
			);
		}break;
		default: __debugbreak(); break;
	}
#endif
}


auto VulkanBuffer::deinit() -> void {
	vkDestroyBuffer(m_device->m_handle, m_buffer, nullptr);
	vkFreeMemory(m_device->m_handle, m_memory, nullptr);

	m_buffer = VK_NULL_HANDLE;
	m_memory = VK_NULL_HANDLE;
}

auto VulkanBuffer::map_to_GPU(void* data, VkDeviceSize size) -> void* {
	VkResult result;

	void* mapped_data;
	result = vkMapMemory(m_device->m_handle, m_memory, 0, size, 0, &mapped_data); if (result != VK_SUCCESS) __debugbreak();
	memcpy(mapped_data, data, static_cast<size_t>(size));
	vkUnmapMemory(m_device->m_handle, m_memory);

	return mapped_data;
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

	VulkanCommandBuffers command_buffer;
	command_buffer.init(*m_device, m_device->m_command_pool, 1);
	command_buffer.record(0,
		[&]() {
			const VkBufferCopy copy_region = {
				.srcOffset = 0,
				.dstOffset = 0,
				.size = size,
			};
			vkCmdCopyBuffer(command_buffer.m_handles[0], src_buffer, dst_buffer, 1, &copy_region);
		}
	);
	const VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = {},
		.waitSemaphoreCount = {},
		.pWaitSemaphores = {},
		.pWaitDstStageMask = {},
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer.m_handles[0],
		.signalSemaphoreCount = {},
		.pSignalSemaphores = {},
	};

	result = vkQueueSubmit(m_device->m_graphics_queue.m_handle, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) __debugbreak();
	result = vkQueueWaitIdle(m_device->m_graphics_queue.m_handle);
	if (result != VK_SUCCESS) __debugbreak();

	command_buffer.deinit();

}
Vulkan_GPUMeshData::Vulkan_GPUMeshData(const VulkanLogicalDevice& device, const Mesh& mesh, VertexFormat vertex_format, bool interleaved) {
	const std::vector<f32> data = convert_into_data(mesh, interleaved);

	m_vertex_buffer.init(device, VULKAN_BUFFER_TYPE::VERTEX, (void*)data.data(), data.size());
	if (mesh.m_indices.size() > 0) {
		m_index_buffer.init(device, VULKAN_BUFFER_TYPE::INDEX, (void*)mesh.m_indices.data(), mesh.m_indices.size());
	}

}
auto Vulkan_GPUMeshData::bind() const -> void {
}
auto Vulkan_GPUMeshData::set_layout(const VertexFormat& vertex_format) -> void {
}
}