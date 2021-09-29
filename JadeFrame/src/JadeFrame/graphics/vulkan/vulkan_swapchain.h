#pragma once
#include <vulkan/vulkan.h>

#include "JadeFrame/defines.h"

#include <vector>


namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanSurface;
class VulkanSemaphore;
class VulkanFence;
class VulkanRenderPass;

class VulkanSwapchain {
public:
	auto init(
		const VulkanLogicalDevice& device, 
		const VulkanPhysicalDevice& physical_device,
		const VulkanSurface& surface
	) -> void;
	auto deinit() -> void;
	auto create_framebuffers(const VulkanRenderPass& render_pass) -> void;

	auto acquire_next_image(const VulkanSemaphore* semaphore, const VulkanFence* fence, VkResult& result) -> u32;
public:
	VkSwapchainKHR m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;

	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_image_views;
	VkFormat m_image_format;
	VkExtent2D m_extent;
	std::vector<VkFramebuffer> m_framebuffers;

	u32 m_width = 0;
	u32 m_height = 0;
};
}