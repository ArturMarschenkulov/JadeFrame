#pragma once
#include <vulkan/vulkan.h>

#include "vulkan_physical_device.h"
#include "vulkan_surface.h"
#include "vulkan_logical_device.h"


#include "JadeFrame/platform/windows/windows_window.h"

#include <vector>





static auto is_device_suitable(VulkanPhysicalDevice physical_device) -> bool {
	bool swapchain_adequate = false;
	if (physical_device.m_extension_support) {
		swapchain_adequate =
			!physical_device.m_swapchain_support_details.m_formats.empty() &&
			!physical_device.m_swapchain_support_details.m_present_modes.empty()
		;
	}
	return
		physical_device.m_queue_family_indices.is_complete() &&
		physical_device.m_extension_support &&
		swapchain_adequate
	;
}


struct VulkanSwapchain {
private:
public:
public:
	VkSwapchainKHR m_swapchain;

	//std::vector<VkImage> m_swapchain_images;
	//VkFormat m_swapchain_image_format;
	//VkExtent2D m_swapchain_extent;
};


//struct VulkanLogicalDevice {
//private:
//	auto create_swapchain(VulkanPhysicalDevice physical_device, VulkanSurface surface, HWND window_handle) -> void;
//	auto create_image_views() -> void;
//	auto create_render_pass() -> void;
//	auto create_graphics_pipeline() -> void;
//	auto create_framebuffers() -> void;
//	auto create_command_pool(VulkanPhysicalDevice physical_device) -> void;
//	auto create_command_buffers() -> void;
//	auto create_sync_objects() -> void;
//
//	auto recreate_swapchain() -> void;
//
//public:
//	auto draw_frame() -> void;
//	auto init(VulkanPhysicalDevice physical_device, VulkanSurface surface, HWND window_handle) -> void;
//	auto deinit() -> void;
//public:
//	VkDevice m_handle;
//	VkQueue m_graphics_queue;
//	VkQueue m_present_queue;
//
//	VkSwapchainKHR m_swapchain;
//	std::vector<VkImage> m_swapchain_images;
//	VkFormat m_swapchain_image_format;
//	VkExtent2D m_swapchain_extent;
//
//	std::vector<VkImageView> m_swapchain_image_views;
//
//	VkRenderPass m_render_pass;
//
//	VkPipelineLayout m_pipeline_layout;
//	VkPipeline m_graphics_pipeline;
//
//	std::vector<VkFramebuffer> m_swapchain_framebuffers;
//
//	VkCommandPool m_command_pool;
//
//	std::vector<VkCommandBuffer> m_command_buffers;
//
//	std::vector<VkSemaphore> m_image_available_semaphores;
//	std::vector<VkSemaphore> m_render_finished_semaphores;
//	std::vector<VkFence> m_in_flight_fences;
//	std::vector<VkFence> m_images_in_flight;
//	size_t m_current_frame = 0;
//};
struct VulkanInstance {

private:
	auto query_layers()->std::vector<VkLayerProperties>;
	auto query_extensions()->std::vector<VkExtensionProperties>;
	auto query_physical_devices() -> std::vector<VulkanPhysicalDevice>;
	auto setup_debug() -> void;
	auto pick_physical_deivce() -> void;
public:
	auto init(HWND window_handle) -> void;
	auto deinit() -> void;
public:
	VkInstance m_instance;

	std::vector<VkLayerProperties> m_layers;

	std::vector<VkExtensionProperties> m_extensions;
	std::vector<const char*> m_extension_names;

	VkDebugUtilsMessengerEXT m_debug_messenger;

	std::vector<VulkanPhysicalDevice> m_physical_devices;
	VulkanPhysicalDevice m_physical_device;

	VulkanLogicalDevice m_logical_device;

	VulkanSurface m_surface;

};

struct Vulkan_Context {
	//Vulkan_Context(const Vulkan_Context&) = delete;
	//Vulkan_Context(Vulkan_Context&&) = delete;
	//auto operator=(const Vulkan_Context&)->Vulkan_Context & = delete;
	//auto operator=(Vulkan_Context&&)->Vulkan_Context & = delete;

	Vulkan_Context() = default;
	Vulkan_Context(Windows_Window* window);
	~Vulkan_Context();
public:
	VulkanInstance m_instance;
	HWND m_window_handle;
public:
	auto main_loop() -> void;
};