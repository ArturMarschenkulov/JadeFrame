#pragma once
#include "vulkan_physical_device.h"
#include "vulkan_surface.h"



struct VulkanLogicalDevice {
private:
	auto create_swapchain(VulkanPhysicalDevice physical_device, VulkanSurface surface, HWND window_handle) -> void;
	auto create_image_views() -> void;
	auto create_render_pass() -> void;
	auto create_graphics_pipeline() -> void;
	auto create_framebuffers() -> void;
	auto create_command_pool(VulkanPhysicalDevice physical_device) -> void;
	auto create_command_buffers() -> void;
	auto create_sync_objects() -> void;

	auto recreate_swapchain() -> void;

public:
	auto draw_frame() -> void;
	auto init(VulkanPhysicalDevice physical_device, VulkanSurface surface, HWND window_handle) -> void;
	auto deinit() -> void;
public:
	VkDevice m_handle;
	VkQueue m_graphics_queue;
	VkQueue m_present_queue;

	VkSwapchainKHR m_swapchain;
	std::vector<VkImage> m_swapchain_images;
	VkFormat m_swapchain_image_format;
	VkExtent2D m_swapchain_extent;

	std::vector<VkImageView> m_swapchain_image_views;

	VkRenderPass m_render_pass;

	VkPipelineLayout m_pipeline_layout;
	VkPipeline m_graphics_pipeline;

	std::vector<VkFramebuffer> m_swapchain_framebuffers;

	VkCommandPool m_command_pool;

	std::vector<VkCommandBuffer> m_command_buffers;

	std::vector<VkSemaphore> m_image_available_semaphores;
	std::vector<VkSemaphore> m_render_finished_semaphores;
	std::vector<VkFence> m_in_flight_fences;
	std::vector<VkFence> m_images_in_flight;
	size_t m_current_frame = 0;
};