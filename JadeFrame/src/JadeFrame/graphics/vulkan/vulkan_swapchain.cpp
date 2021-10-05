#include "pch.h"
#include "vulkan_swapchain.h"

#include <Windows.h> // TODO: Try to remove it


#include "vulkan_logical_device.h"
#include "vulkan_context.h"
#include "vulkan_render_pass.h"
#include "vulkan_sync_object.h"

#undef min
#undef max

namespace JadeFrame {

static auto choose_surface_format(const std::vector<VkSurfaceFormatKHR>& available_surface_formats) ->VkSurfaceFormatKHR {
	for (u32 i = 0; i < available_surface_formats.size(); i++) {
		if (available_surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			available_surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return available_surface_formats[i];
		}
	}
	return available_surface_formats[0];

}
static auto choose_present_mode(const std::vector<VkPresentModeKHR>& available_surface_formats) -> VkPresentModeKHR {
	std::array<VkPresentModeKHR, 3> mode_ranks = {
		VK_PRESENT_MODE_FIFO_KHR, 
		VK_PRESENT_MODE_MAILBOX_KHR,
		VK_PRESENT_MODE_IMMEDIATE_KHR
	};
	for (u32 i = 0; i < available_surface_formats.size(); i++) {
		for(u32 j = 0; j < mode_ranks.size(); j++) {
			if(available_surface_formats[i] == mode_ranks[j]) {
				const VkPresentModeKHR best_mode = available_surface_formats[i];
				return best_mode;
			}
		}
	}
	assert(!"Should not reach here!");
}
static auto choose_extent(const VkSurfaceCapabilitiesKHR& available_capabilities, const VulkanSwapchain& swapchain) -> VkExtent2D {
		//vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_handle, surface.m_surface, &m_surface_capabilities);
	if (false/*m_surface_capabilities.currentExtent.width != UINT32_MAX*/) {
		return available_capabilities.currentExtent;
	} else {
		RECT area;
		const HWND& wh = swapchain.m_device->m_physical_device_p->m_instance_p->m_window_handle;
		GetClientRect(wh, &area);
		i32 width = area.right;
		i32 height = area.bottom;
		//glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actual_extent = {
			static_cast<u32>(width),
			static_cast<u32>(height)
		};

		//actual_extent.width = std::max(m_surface_capabilities.minImageExtent.width, std::min(m_surface_capabilities.maxImageExtent.width, actual_extent.width));
		//actual_extent.height = std::max(m_surface_capabilities.minImageExtent.height, std::min(m_surface_capabilities.maxImageExtent.height, actual_extent.height));


		//actual_extent.width = std::clamp(actual_extent.width, m_surface_capabilities.minImageExtent.width, m_surface_capabilities.maxImageExtent.width);
		//actual_extent.height = std::clamp(actual_extent.height, m_surface_capabilities.minImageExtent.height, m_surface_capabilities.maxImageExtent.height);

		return actual_extent;
	}
}
auto VulkanSwapchain::init(
	const VulkanLogicalDevice& device, 
	const VulkanPhysicalDevice& physical_device,
	const VulkanSurface& surface
) -> void {
	m_device = &device;
	VkResult result;
	const SurfaceSupportDetails& surface_details = physical_device.m_surface_support_details;

	u32 image_count = surface_details.m_capabilities.minImageCount + 1;
	if (surface_details.m_capabilities.maxImageCount > 0 && image_count > surface_details.m_capabilities.maxImageCount) {
		image_count = surface_details.m_capabilities.maxImageCount;
	}

	const VkSurfaceFormatKHR surface_format = choose_surface_format(surface_details.m_formats);
	const VkPresentModeKHR present_mode = choose_present_mode(surface_details.m_present_modes);
	const VkExtent2D extent = choose_extent(surface_details.m_capabilities, *this);
	m_image_format = surface_format.format;
	m_extent = extent;

	const QueueFamilyIndices& indices = physical_device.m_queue_family_indices;
	const u32 queue_family_indices[] = {
		indices.m_graphics_family.value(),
		indices.m_present_family.value()
	};

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.pNext = nullptr;
	create_info.flags = 0;
	create_info.surface = surface.m_handle;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = nullptr;
	create_info.preTransform = physical_device.m_surface_support_details.m_capabilities.currentTransform;
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




	std::vector<VkImage> images;
	result = vkGetSwapchainImagesKHR(device.m_handle, m_handle, &image_count, nullptr);
	if (VK_SUCCESS != result) __debugbreak();
	images.resize(image_count);
	result = vkGetSwapchainImagesKHR(device.m_handle, m_handle, &image_count, images.data());
	if (VK_SUCCESS != result) __debugbreak();

	m_images.resize(image_count);
	for (u32 i = 0; i < m_images.size(); i++) {
		m_images[i].init(device, images[i]);
	}
	
	m_image_views.resize(image_count);
	for (u32 i = 0; i < m_images.size(); i++) {
		m_image_views[i].init(device, m_images[i], surface_format.format);
	}
}

auto VulkanSwapchain::deinit() -> void {
	for (uint32_t i = 0; i < m_framebuffers.size(); i++) {
		m_framebuffers[i].deinit();
	}
	for (uint32_t i = 0; i < m_image_views.size(); i++) {
		m_image_views[i].deinit();
	}

	vkDestroySwapchainKHR(m_device->m_handle, m_handle, nullptr);
}

auto VulkanSwapchain::create_framebuffers(const VulkanRenderPass& render_pass) -> void {

	m_framebuffers.resize(m_image_views.size());
	for (size_t i = 0; i < m_image_views.size(); i++) {
		m_framebuffers[i].init(*m_device, m_image_views[i], render_pass, m_extent);
	}
}

auto VulkanSwapchain::acquire_next_image(const VulkanSemaphore* semaphore, const VulkanFence* fence, VkResult& out_result) -> u32 {
	u32 image_index;
	out_result = vkAcquireNextImageKHR(
		m_device->m_handle, 
		m_handle, 
		UINT64_MAX, 
		semaphore == nullptr ? VK_NULL_HANDLE : semaphore->m_handle,
		fence == nullptr ? VK_NULL_HANDLE : fence->m_handle, 
		&image_index
	);
	return image_index;

}

auto VulkanFramebuffer::init(
	const VulkanLogicalDevice& device,
	const VulkanImageView& image_view,
	const VulkanRenderPass& render_pass,
	VkExtent2D extent
) -> void {
	m_device = &device;
	m_image_view = &image_view;
	m_render_pass = &render_pass;
	VkResult result;
	std::array<VkImageView, 1> attachments = {
		image_view.m_handle
	};

	VkFramebufferCreateInfo framebuffer_info{};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.pNext = nullptr;
	framebuffer_info.flags = 0;
	framebuffer_info.renderPass = render_pass.m_handle;
	framebuffer_info.attachmentCount = attachments.size();
	framebuffer_info.pAttachments = attachments.data();
	framebuffer_info.width = extent.width;
	framebuffer_info.height = extent.height;
	framebuffer_info.layers = 1;

	result = vkCreateFramebuffer(device.m_handle, &framebuffer_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();
}

auto VulkanFramebuffer::deinit() -> void {
	vkDestroyFramebuffer(m_device->m_handle, m_handle, nullptr);
}

}