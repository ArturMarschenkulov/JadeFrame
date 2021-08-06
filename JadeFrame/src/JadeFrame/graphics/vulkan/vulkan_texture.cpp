#include "vulkan_texture.h"
#include "JadeFrame/defines.h"

#include "vulkan_buffer.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <string>

namespace JadeFrame {

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

		VulkanBuffer staging_buffer = { VULKAN_BUFFER_TYPE::STAGING };
		staging_buffer.init(*this, VULKAN_BUFFER_TYPE::STAGING, nullptr, image_size);

		void* mapped_data;
		result = vkMapMemory(m_handle, staging_buffer.m_memory, 0, image_size, 0, &mapped_data);
		if (result != VK_SUCCESS) __debugbreak();
		memcpy(mapped_data, image.data, static_cast<size_t>(image_size));
		vkUnmapMemory(m_handle, staging_buffer.m_memory);


		this->create_image(image.width, image.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_texture_image, m_texture_image_Memory);

		this->transition_image_layout(m_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		this->copy_buffer_to_image(staging_buffer.m_buffer, m_texture_image, static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height));
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
		.memoryTypeIndex = find_memory_type(*m_physical_device_p, mem_requirements.memoryTypeBits, properties),
	};
	result = vkAllocateMemory(m_handle, &alloc_info, nullptr, &image_memory);
	if (result != VK_SUCCESS) __debugbreak();

	result = vkBindImageMemory(m_handle, image, image_memory, 0);
	if (result != VK_SUCCESS) __debugbreak();
}
auto VulkanLogicalDevice::transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) -> void {
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
		.pNext = {},
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

	result = vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) __debugbreak();
	result = vkQueueWaitIdle(m_graphics_queue);
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


#if 0


auto create_texture_image() -> void {

	STBIImage image("textures/texture.jpg");
	VkDeviceSize image_size = image.width * image.height * 4;

	if (!image.data) {
		//throw std::runtime_error("failed to load texture image!");
	}

	VulkanBuffer staging_buffer = { VULKAN_BUFFER_TYPE::STAGING };
	staging_buffer.init(device, VULKAN_BUFFER_TYPE::STAGING, nullptr, image_size);

	void* mapped_data;
	vkMapMemory(device, staging_buffer.m_memory, 0, image_size, 0, &mapped_data);
	memcpy(mapped_data, image.data, static_cast<size_t>(image_size));
	vkUnmapMemory(device, staging_buffer.m_memory);

	stbi_image_free(image.data);

	createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	staging_buffer.deinit();
}
#endif
}