#include "vulkan_buffer.h"

#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"

#include <iostream>
#include "JadeFrame/math/mat_4.h"

namespace JadeFrame {

static auto find_memory_type(const VulkanPhysicalDevice& physical_device, u32 type_filter, VkMemoryPropertyFlags properties) -> u32 {
	const VkPhysicalDeviceMemoryProperties& mem_props = physical_device.m_memory_properties;
	for (u32 i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");
}


VulkanBuffer::VulkanBuffer(const VULKAN_BUFFER_TYPE type)
	: m_type(type) {
}

auto VulkanBuffer::init(const VulkanLogicalDevice& device, VULKAN_BUFFER_TYPE buffer_type, void* data, size_t size) -> void {
	VkResult result;
	m_device = &device;
#if 0
	switch(buffer_type) {
		//with staging buffer
		case VULKAN_BUFFER_TYPE::VERTEX:
		case VULKAN_BUFFER_TYPE::INDEX:
		{

			VulkanBuffer staging_buffer = { VULKAN_BUFFER_TYPE::STAGING };
			staging_buffer.init(device, VULKAN_BUFFER_TYPE::STAGING, nullptr, size);

			void* mapped_data;
			result = vkMapMemory(device.m_handle, staging_buffer.m_memory, 0, size, 0, &mapped_data);
			if (result != VK_SUCCESS) __debugbreak();
			memcpy(mapped_data, data, static_cast<size_t>(size));
			vkUnmapMemory(device.m_handle, staging_buffer.m_memory);

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

		void* mapped_data;
		result = vkMapMemory(device.m_handle, staging_buffer.m_memory, 0, size, 0, &mapped_data);
		if (result != VK_SUCCESS) __debugbreak();
		memcpy(mapped_data, data, static_cast<size_t>(size));
		vkUnmapMemory(device.m_handle, staging_buffer.m_memory);

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

			void* mapped_data;
			result = vkMapMemory(device.m_handle, staging_buffer.m_memory, 0, size, 0, &mapped_data);
			if (result != VK_SUCCESS) __debugbreak();
			memcpy(mapped_data, data, static_cast<size_t>(size));
			vkUnmapMemory(device.m_handle, staging_buffer.m_memory);


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

			void* mapped_data;
			result = vkMapMemory(device.m_handle, staging_buffer.m_memory, 0, size, 0, &mapped_data);
			if (result != VK_SUCCESS) __debugbreak();
			memcpy(mapped_data, data, static_cast<size_t>(size));
			vkUnmapMemory(device.m_handle, staging_buffer.m_memory);



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
		.memoryTypeIndex = find_memory_type(*m_device->m_physical_device_p, mem_requirements.memoryTypeBits, properties),
	};

	result = vkAllocateMemory(m_device->m_handle, &alloc_info, nullptr, &buffer_memory);
	if (result != VK_SUCCESS) __debugbreak();

	result = vkBindBufferMemory(m_device->m_handle, buffer, buffer_memory, 0);
}

auto VulkanBuffer::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void {
	VkResult result;

	const VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = {},
		.commandPool = m_device->m_command_pool.m_handle,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer command_buffer;
	result = vkAllocateCommandBuffers(m_device->m_handle, &alloc_info, &command_buffer);
	if (result != VK_SUCCESS) __debugbreak();

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = {},
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = {},
	};

	result = vkBeginCommandBuffer(command_buffer, &begin_info);
	if (result != VK_SUCCESS) __debugbreak();

	const VkBufferCopy copy_region = {
		.srcOffset = 0,
		.dstOffset = 0,
		.size = size,
	};
	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

	result = vkEndCommandBuffer(command_buffer);
	if (result != VK_SUCCESS) __debugbreak();

	const VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = {},
		.waitSemaphoreCount = {},
		.pWaitSemaphores = {},
		.pWaitDstStageMask = {},
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
		.signalSemaphoreCount = {},
		.pSignalSemaphores = {},
	};

	result = vkQueueSubmit(m_device->m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) __debugbreak();
	result = vkQueueWaitIdle(m_device->m_graphics_queue);
	if (result != VK_SUCCESS) __debugbreak();

	vkFreeCommandBuffers(m_device->m_handle, m_device->m_command_pool.m_handle, 1, &command_buffer);
}
}