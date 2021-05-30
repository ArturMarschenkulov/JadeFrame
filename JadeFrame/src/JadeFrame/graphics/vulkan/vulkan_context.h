#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "JadeFrame/platform/windows/windows_window.h"

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR m_capabilities;
	std::vector<VkSurfaceFormatKHR> m_formats;
	std::vector<VkPresentModeKHR> m_present_modes;
};
struct HWND__;	typedef HWND__* HWND;

struct VulkanInstance {
	VkInstance m_instance;
	std::vector<VkLayerProperties> m_layers;
	std::vector<VkExtensionProperties> m_extensions;
	std::vector<const char*> m_extension_names;
	VkDebugUtilsMessengerEXT m_debug_messenger;

	auto query_layers()->std::vector<VkLayerProperties>;
	auto query_extensions()->std::vector<VkExtensionProperties>;
	auto query_physical_devices() -> std::vector<VkPhysicalDevice>;
	auto setup_debug() -> void;
	auto init() -> void;
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
	//VkInstance m_instance;
	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
	VkDevice m_device;
	VkSwapchainKHR m_swapchain;
	VkPipelineLayout m_pipeline_layout;
	VkRenderPass m_render_pass;
	VkPipeline m_graphics_pipeline;
	std::vector<VkCommandBuffer> m_command_buffers;

	std::vector<VkFramebuffer> m_swapchain_framebuffers;
	VkCommandPool m_command_pool;
	std::vector<VkExtensionProperties> m_extensions;
	std::vector<const char*> m_extension_names;
	std::vector<VkLayerProperties> m_layers;
	std::vector<VkPhysicalDevice> m_physical_devices;

	VkQueue m_graphics_queue;
	VkQueue m_present_queue;
	VkSurfaceKHR m_surface;
	std::vector<VkImage> m_swapchain_images;
	VkFormat m_swapchain_image_format;
	VkExtent2D m_swapchain_extent;
	
	std::vector<VkImageView> m_swapchain_image_views;

	std::vector<VkSemaphore> m_image_available_semaphores;
	std::vector<VkSemaphore> m_render_finished_semaphores;
	std::vector<VkFence> m_in_flight_fences;
	std::vector<VkFence> m_images_in_flight;
	size_t m_current_frame = 0;

public:

	auto create_instance() -> void;
	auto setup_debug_messenger() -> void;
	auto create_surface() -> void;
	auto pick_physical_device() -> void;
	auto create_logical_device() -> void;
	auto create_swapchain() -> void;
	auto create_image_views() -> void;
	auto create_graphics_pipeline() -> void;
	auto create_render_pass() -> void;

	auto create_framebuffers() -> void;
	auto create_command_pool() -> void;
	auto create_command_buffers() -> void;

	auto main_loop() -> void;
	auto draw_frame() -> void;

	auto create_sync_objects() -> void;

	//auto query_extensions() -> std::vector<VkExtensionProperties>;
	//auto query_physical_devices() -> std::vector<VkPhysicalDevice>;
	//auto query_layers() -> std::vector<VkLayerProperties>;
	//auto query_swapchain_support(VkPhysicalDevice device)->SwapChainSupportDetails;
};