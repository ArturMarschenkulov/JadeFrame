#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {

class VulkanLogicalDevice;

class VulkanImage {
public:
	auto init(const VulkanLogicalDevice& device) -> void;
	auto deinit() -> void;

	VkImage m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;
};

class VulkanImageView {
public:
	auto init(const VulkanLogicalDevice& device) -> void;
	auto deinit() -> void;

	VkImageView m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;
};
class VulkanTexture {

};

}