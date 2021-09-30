#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {

class VulkanLogicalDevice;

class VulkanImage {
public:
	enum class SOURCE {
		REGULAR,
		SWAPCHAIN
	};
	auto init(const VulkanLogicalDevice& device, VkImage image) -> void;
	auto deinit() -> void;

	VkImage m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;
	SOURCE m_source;
};

class VulkanImageView {
public:
	auto init(const VulkanLogicalDevice& device, const VulkanImage& image, VkFormat format) -> void;
	auto deinit() -> void;
public:
	VkImageView m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;
	const VulkanImage* m_image = nullptr;
};


class VulkanTexture {

};

}