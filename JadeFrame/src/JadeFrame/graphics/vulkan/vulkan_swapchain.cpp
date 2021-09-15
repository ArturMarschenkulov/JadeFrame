#include "pch.h"
#include "vulkan_swapchain.h"

#include <Windows.h> // TODO: Try to remove it


#include "vulkan_logical_device.h"
#include "vulkan_context.h"

#undef min
#undef max

namespace JadeFrame {

static auto create_image_views(const VulkanLogicalDevice& device, std::vector<VkImage>& swapchain_images, VkFormat image_format) -> std::vector<VkImageView> {
	VkResult result;

	std::vector<VkImageView> swapchain_image_views;
	swapchain_image_views.resize(swapchain_images.size());

	for (size_t i = 0; i < swapchain_images.size(); i++) {
		VkImageViewCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchain_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = image_format,
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

		result = vkCreateImageView(device.m_handle, &create_info, nullptr, &swapchain_image_views[i]);
		if (result != VK_SUCCESS) __debugbreak();
	}
	return swapchain_image_views;
}
auto VulkanSwapchain::init(
	const VulkanLogicalDevice& device, 
	const VulkanPhysicalDevice& physical_device,
	const VulkanSurface& surface
) -> void {
	VkResult result;

	//const VulkanPhysicalDevice& pd = *device.m_physical_device_p;
	//const VulkanInstance& instance = *device.m_instance_p;

	u32 image_count = physical_device.m_surface_capabilities.minImageCount + 1;
	if (physical_device.m_surface_capabilities.maxImageCount > 0 && image_count > physical_device.m_surface_capabilities.maxImageCount) {
		image_count = physical_device.m_surface_capabilities.maxImageCount;
	}

	const VkSurfaceFormatKHR surface_format = physical_device.choose_swap_surface_format();
	const VkPresentModeKHR present_mode = physical_device.choose_swap_present_mode();
	const VkExtent2D extent = physical_device.choose_swap_extent();

	const QueueFamilyIndices& indices = physical_device.m_queue_family_indices;
	const u32 queue_family_indices[] = {
		indices.m_graphics_family.value(),
		indices.m_present_family.value()
	};

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.pNext = nullptr;
	create_info.flags = 0;
	create_info.surface = surface.m_surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = nullptr;
	create_info.preTransform = physical_device.m_surface_capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;
	if (indices.m_graphics_family != indices.m_present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}



	result = vkCreateSwapchainKHR(device.m_handle, &create_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();

	result = vkGetSwapchainImagesKHR(device.m_handle, m_handle, &image_count, nullptr);
	m_images.resize(image_count);
	result = vkGetSwapchainImagesKHR(device.m_handle, m_handle, &image_count, m_images.data());
	if (VK_SUCCESS != result) __debugbreak();

	m_image_format = surface_format.format;
	m_extent = extent;
	m_image_views = create_image_views(device, m_images, surface_format.format);

	m_device = &device;

}

auto VulkanSwapchain::deinit() -> void {
	for (uint32_t i = 0; i < m_framebuffers.size(); i++) {
		vkDestroyFramebuffer(m_device->m_handle, m_framebuffers[i], nullptr);
	}
	for (uint32_t i = 0; i < m_image_views.size(); i++) {
		vkDestroyImageView(m_device->m_handle, m_image_views[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device->m_handle, m_handle, nullptr);

	m_height = 0;
	m_width = 0;
}

auto VulkanSwapchain::create_framebuffers(const VkRenderPass& render_pass) -> void {
	VkResult result;

	m_framebuffers.resize(m_image_views.size());

	for (size_t i = 0; i < m_image_views.size(); i++) {
		VkImageView attachments[] = {
			m_image_views[i]
		};

		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.pNext = nullptr;
		framebuffer_info.flags = 0;
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = m_extent.width;
		framebuffer_info.height = m_extent.height;
		framebuffer_info.layers = 1;

		result = vkCreateFramebuffer(m_device->m_handle, &framebuffer_info, nullptr, &m_framebuffers[i]);
		if (result != VK_SUCCESS) __debugbreak();
	}
}

}