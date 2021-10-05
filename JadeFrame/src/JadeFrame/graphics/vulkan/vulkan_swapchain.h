#pragma once
#include <vulkan/vulkan.h>

#include "JadeFrame/defines.h"
#include "vulkan_texture.h"

#include <vector>


namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanSurface;
class VulkanSemaphore;
class VulkanFence;
class VulkanRenderPass;
class VulkanImageView;


class VulkanFramebuffer {
public:
	auto init(
		const VulkanLogicalDevice& device,
		const VulkanImageView& image_view,
		const VulkanRenderPass& render_pass,
		VkExtent2D extent
	) -> void;
	auto deinit() -> void;
public:
	VkFramebuffer m_handle;
	const VulkanLogicalDevice* m_device = nullptr;
	const VulkanImageView* m_image_view = nullptr;
	const VulkanRenderPass* m_render_pass = nullptr;
	//const VulkanSwapchain* m_swapchain = nullptr;

};


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

	std::vector<VulkanImage> m_images;
	std::vector<VulkanImageView> m_image_views;

	VkFormat m_image_format;
	VkExtent2D m_extent;
	std::vector<VulkanFramebuffer> m_framebuffers;
};
}