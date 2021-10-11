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


		void* mapped_data = staging_buffer.map_to_GPU(data, 0, size);

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

auto VulkanBuffer::map_to_GPU(void* data, VkDeviceSize offset, VkDeviceSize size) -> void* {
	VkResult result;

	void* mapped_data;
	result = vkMapMemory(m_device->m_handle, m_memory, offset, size, 0, &mapped_data);
	if (result != VK_SUCCESS) __debugbreak();
	memcpy(mapped_data, data, static_cast<size_t>(size));
	vkUnmapMemory(m_device->m_handle, m_memory);

	return mapped_data;
}

auto VulkanBuffer::resize(size_t size) -> void {
	assert(m_type == TYPE::UNIFORM);
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

#include "stb/stb_image.h"

struct STBIImage {
	STBIImage(const std::string& path) {
		// flip textures on their y coordinate while loading
		stbi_set_flip_vertically_on_load(true);
		//i32 width, height, num_components;
		data = stbi_load(path.c_str(), &width, &height, &num_components, 0); // STBI_rgb_alpha
	}
	~STBIImage() {
		stbi_image_free(data);
	}
	i32 width;
	i32 height;
	i32 num_components;
	unsigned char* data;
};

auto VulkanLogicalDevice::create_texture_image(const std::string& path) -> void {
	VkResult result;

	STBIImage image(path);


	if (image.data) {
		VkDeviceSize image_size = image.width * image.height * image.num_components;

		VulkanBuffer staging_buffer = { VulkanBuffer::TYPE::STAGING };
		staging_buffer.init(*this, VulkanBuffer::TYPE::STAGING, nullptr, image_size);

		void* mapped_data = staging_buffer.map_to_GPU(image.data, 0, image_size);

		this->create_image(
			image.width, image.height,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_texture_image, m_texture_image_Memory
		);

		this->transition_image_layout(
			m_texture_image,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		this->copy_buffer_to_image(staging_buffer.m_handle, m_texture_image, static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height));
		this->transition_image_layout(m_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		staging_buffer.deinit();
	} else {
		__debugbreak();
	}
}
auto VulkanLogicalDevice::create_image(u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory) -> void {
	VkResult result;
	const VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = {
			.width = width,
			.height = height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = tiling,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = {},
		.pQueueFamilyIndices = {},
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	result = vkCreateImage(m_handle, &image_info, nullptr, &image);
	if (result != VK_SUCCESS) __debugbreak();


	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(m_handle, image, &mem_requirements);

	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = {},
		.allocationSize = mem_requirements.size,
		.memoryTypeIndex = m_physical_device_p->find_memory_type(mem_requirements.memoryTypeBits, properties),
	};
	result = vkAllocateMemory(m_handle, &alloc_info, nullptr, &image_memory);
	if (result != VK_SUCCESS) __debugbreak();

	result = vkBindImageMemory(m_handle, image, image_memory, 0);
	if (result != VK_SUCCESS) __debugbreak();
}
auto VulkanLogicalDevice::transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) -> void {
	//VulkanCommandBuffer command_buffer_ = m_command_pool.allocate_command_buffers(1)[0];
	VkCommandBuffer command_buffer = this->begin_single_time_commands();
	{
		VkImageMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.oldLayout = old_layout,
			.newLayout = new_layout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
		};

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;

		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else {
			__debugbreak();
		}

		vkCmdPipelineBarrier(
			command_buffer,
			source_stage, destination_stage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}
	this->end_single_time_commands(command_buffer);
}
auto VulkanLogicalDevice::copy_buffer_to_image(VkBuffer buffer, VkImage image, u32 width, u32 height) -> void {
	VkCommandBuffer command_buffer = this->begin_single_time_commands();
	{
		const VkBufferImageCopy region = {
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = {
				width,
				height,
				1
			},
		};

		vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
	this->end_single_time_commands(command_buffer);
}

auto VulkanLogicalDevice::begin_single_time_commands() -> VkCommandBuffer {
	VkResult result;
	const VkCommandBufferAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = m_command_pool.m_handle,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer command_buffer;
	result = vkAllocateCommandBuffers(m_handle, &alloc_info, &command_buffer);
	if (result != VK_SUCCESS) __debugbreak();

	const VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = {},
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = {},
	};

	result = vkBeginCommandBuffer(command_buffer, &begin_info);
	if (result != VK_SUCCESS) __debugbreak();

	return command_buffer;
}

auto VulkanLogicalDevice::end_single_time_commands(VkCommandBuffer command_buffer) -> void {
	VkResult result;

	result = vkEndCommandBuffer(command_buffer);
	if (result != VK_SUCCESS) __debugbreak();

	const VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
	};

	result = vkQueueSubmit(m_graphics_queue.m_handle, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) __debugbreak();
	result = vkQueueWaitIdle(m_graphics_queue.m_handle);
	if (result != VK_SUCCESS) __debugbreak();

	vkFreeCommandBuffers(m_handle, m_command_pool.m_handle, 1, &command_buffer);
}

auto VulkanLogicalDevice::create_texture_image_view() -> void {
	m_texture_image_view = this->create_image_view(m_texture_image, VK_FORMAT_R8G8B8A8_SRGB);
}

auto VulkanLogicalDevice::create_image_view(VkImage image, VkFormat format) -> VkImageView {
	VkResult result;

	const VkImageViewCreateInfo view_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	VkImageView image_view;
	result = vkCreateImageView(m_handle, &view_info, nullptr, &image_view);
	if (result != VK_SUCCESS) __debugbreak();

	return image_view;
}

auto VulkanLogicalDevice::create_texture_sampler() -> void {
	VkResult result;

	const VkSamplerCreateInfo samplerInfo = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = {},
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = m_physical_device_p->m_properties.limits.maxSamplerAnisotropy,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = {},
		.maxLod = {},
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
	};
	result = vkCreateSampler(m_handle, &samplerInfo, nullptr, &m_texture_sampler);
	if (result != VK_SUCCESS) __debugbreak();
}

auto VulkanImage::init(const VulkanLogicalDevice& device, VkImage image) -> void {
	m_device = &device;
	m_handle = image;
	m_source = SOURCE::SWAPCHAIN;
}
auto VulkanImage::deinit() -> void {
	switch (m_source) {
		case SOURCE::REGULAR:
		{
			__debugbreak();
		}break;
		case SOURCE::SWAPCHAIN:
		{

		}break;
		default: __debugbreak();
	}
}

auto VulkanImageView::init(const VulkanLogicalDevice& device, const VulkanImage& image, VkFormat format) -> void {
	m_device = &device;
	m_image = &image;
	VkResult result;
	VkImageViewCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image.m_handle,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.components = {
			.r = VK_COMPONENT_SWIZZLE_R,
			.g = VK_COMPONENT_SWIZZLE_G,
			.b = VK_COMPONENT_SWIZZLE_B,
			.a = VK_COMPONENT_SWIZZLE_A,
		},
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};

	result = vkCreateImageView(device.m_handle, &create_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();
}
auto VulkanImageView::deinit() -> void {
	vkDestroyImageView(m_device->m_handle, m_handle, nullptr);
}
}