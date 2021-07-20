#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {
class VulkanLogicalDevice;

class VulkanRenderPass {
public:
	auto init(const VulkanLogicalDevice& device, VkFormat image_format) -> void;
	auto deinit() -> void;
public:
	VkRenderPass m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;
};
}