#pragma once
#include <vulkan/vulkan.h>

#include "JadeFrame/defines.h"

#include <vector>

class VulkanLogicalDevice;
class VulkanInstance;

class VulkanSwapchain {
public:
	auto init(const VulkanLogicalDevice& device, const VulkanInstance& instance) -> void;
	auto deinit() -> void;
	auto create_framebuffers(const VkRenderPass& render_pass) -> void;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;

	std::vector<VkImage> m_swapchain_images;
	std::vector<VkImageView> m_swapchain_image_views;
	VkFormat m_swapchain_image_format;
	VkExtent2D m_swapchain_extent;
	std::vector<VkFramebuffer> m_framebuffers;

	u32 m_width = 0;
	u32 m_height = 0;
};