#pragma once
#include <vulkan/vulkan.h>

class VulkanLogicalDevice;

class VulkanRenderPass {
public:
	auto init(const VulkanLogicalDevice& device) -> void;
public:
	VkRenderPass m_render_pass = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
};