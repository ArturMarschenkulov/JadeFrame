#include "vulkan_swapchain.h"

#include <Windows.h> // TODO: Try to remove it
#include <iostream>

#include "vulkan_logical_device.h"
#include "vulkan_context.h"

#undef min
#undef max

static auto choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) -> VkSurfaceFormatKHR {
	for (u32 i = 0; i < available_formats.size(); i++) {
		if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return available_formats[i];
		}
	}
	return available_formats[0];
}
static auto choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) -> VkPresentModeKHR {
	for (u32 i = 0; i < available_present_modes.size(); i++) {
		if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return available_present_modes[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

static auto choose_swap_extent(HWND window_handle, const VkSurfaceCapabilitiesKHR& capabilities) -> VkExtent2D {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		RECT area;
		GetClientRect(window_handle, &area);
		i32 width = area.right;
		i32 height = area.bottom;
		//glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actual_extent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

		return actual_extent;
	}
}

static auto create_image_views(const VulkanLogicalDevice& device, std::vector<VkImage>& swapchain_images) -> std::vector<VkImageView> {
	VkResult result;

	std::vector<VkImageView> swapchain_image_views;
	swapchain_image_views.resize(swapchain_images.size());

	for (size_t i = 0; i < swapchain_images.size(); i++) {
		VkImageViewCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = swapchain_images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = device.m_swapchain.m_swapchain_image_format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_R;
		create_info.components.g = VK_COMPONENT_SWIZZLE_G;
		create_info.components.b = VK_COMPONENT_SWIZZLE_B;
		create_info.components.a = VK_COMPONENT_SWIZZLE_A;
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		result = vkCreateImageView(device.m_handle, &create_info, nullptr, &swapchain_image_views[i]);
		if (result != VK_SUCCESS) {

			__debugbreak();
			//throw std::runtime_error("failed to create image views!");
		}
	}
	return swapchain_image_views;
}
auto VulkanSwapchain::init(const VulkanLogicalDevice& device, const VulkanInstance& instance) -> void {
	VkResult result;

	const VulkanPhysicalDevice& pd = *device.m_physical_device_p;

	u32 image_count = pd.m_surface_capabilities.minImageCount + 1;
	if (pd.m_surface_capabilities.maxImageCount > 0 && image_count > pd.m_surface_capabilities.maxImageCount) {
		image_count = pd.m_surface_capabilities.maxImageCount;
	}

	const VkSurfaceFormatKHR surface_format = choose_swap_surface_format(pd.m_surface_formats);
	const VkPresentModeKHR present_mode = choose_swap_present_mode(pd.m_present_modes);
	const VkExtent2D extent = choose_swap_extent(instance.m_window_handle, pd.m_surface_capabilities);

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.pNext = nullptr;
	create_info.flags = 0;
	create_info.surface = instance.m_surface.m_surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = nullptr;
	create_info.preTransform = pd.m_surface_capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;


	const QueueFamilyIndices& indices = instance.m_physical_device.m_queue_family_indices;
	u32 queue_family_indices[] = {
		indices.m_graphics_family.value(),
		indices.m_present_family.value()
	};

	if (indices.m_graphics_family != indices.m_present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}



	result = vkCreateSwapchainKHR(device.m_handle, &create_info, nullptr, &m_swapchain);
	if (result != VK_SUCCESS) {
		std::cout << "failed to create swap chain!" << std::endl;
		__debugbreak();
	}

	result = vkGetSwapchainImagesKHR(device.m_handle, m_swapchain, &image_count, nullptr);
	m_swapchain_images.resize(image_count);
	result = vkGetSwapchainImagesKHR(device.m_handle, m_swapchain, &image_count, m_swapchain_images.data());
	if (VK_SUCCESS != result) {
		__debugbreak();
	}
	m_swapchain_image_format = surface_format.format;
	m_swapchain_extent = extent;

	m_swapchain_image_views = create_image_views(device, m_swapchain_images);

	m_device = device.m_handle;

}

auto VulkanSwapchain::deinit() -> void {
	for (uint32_t i = 0; i < m_framebuffers.size(); i++) {
		vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
	}
	for (uint32_t i = 0; i < m_swapchain_image_views.size(); i++) {
		vkDestroyImageView(m_device, m_swapchain_image_views[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

	m_height = 0;
	m_width = 0;
}

auto VulkanSwapchain::create_framebuffers(const VkRenderPass& render_pass) -> void {
	VkResult result;

	m_framebuffers.resize(m_swapchain_image_views.size());

	for (size_t i = 0; i < m_swapchain_image_views.size(); i++) {
		VkImageView attachments[] = {
			m_swapchain_image_views[i]
		};

		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.pNext = nullptr;
		framebuffer_info.flags = 0;
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = m_swapchain_extent.width;
		framebuffer_info.height = m_swapchain_extent.height;
		framebuffer_info.layers = 1;

		result = vkCreateFramebuffer(m_device, &framebuffer_info, nullptr, &m_framebuffers[i]);
		if (result != VK_SUCCESS) {
			__debugbreak();
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}
