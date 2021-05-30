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

struct VulkanSurface {
private:
public:
	auto init(VkInstance instance, HWND window_handle) -> void;
public:
	VkSurfaceKHR m_surface;
};
struct VulkanPhysicalDevice {
private:
public:
public:
	VkPhysicalDevice m_physical_device;
};
struct VulkanLogicalDevice {
private:
	auto create_swapchain(VulkanPhysicalDevice physical_device, VulkanSurface surface, HWND window_handle) -> void;
	auto create_image_views() -> void;
	auto create_render_pass() -> void;
	auto create_graphics_pipeline() -> void;
	auto create_framebuffers() -> void;
	auto create_command_pool(VulkanPhysicalDevice physical_device, VulkanSurface surface) -> void;
	auto create_command_buffers() -> void;
	auto create_sync_objects() -> void;

public:
	auto draw_frame() -> void;
	auto init(VulkanPhysicalDevice physical_device, VulkanSurface surface, HWND window_handle) -> void;
	auto deinit() -> void;
public:
	VkDevice m_device;
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
struct VulkanInstance {

private:
	auto query_layers()->std::vector<VkLayerProperties>;
	auto query_extensions()->std::vector<VkExtensionProperties>;
	auto query_physical_devices() -> std::vector<VulkanPhysicalDevice>;
	auto setup_debug() -> void;
	auto pick_physical_deivce() -> void;
	auto create_surface(HWND window_handle) -> void;
public:
	auto find_queue_families(VulkanPhysicalDevice physical_device, VkSurfaceKHR surface);
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

	VulkanLogicalDevice m_device;

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