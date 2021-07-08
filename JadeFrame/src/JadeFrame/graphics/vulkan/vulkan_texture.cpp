#include "vulkan_texture.h"
#include "JadeFrame/defines.h"

#include "vulkan_buffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <string>

namespace JadeFrame {
#if 0
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
	i32 width, height, num_components;
	unsigned char* data;
};

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