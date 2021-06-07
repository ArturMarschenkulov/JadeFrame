#pragma once
#include "vulkan_physical_device.h"
#include "vulkan_surface.h"
#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/vec_3.h"
#include <array>
struct VVertex {
	Vec2 pos;
	Vec3 color;

	static auto get_binding_description() -> VkVertexInputBindingDescription {
		VkVertexInputBindingDescription binding_description = {};
		binding_description.binding = 0;
		binding_description.stride = sizeof(VVertex);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_description;
	}

	static auto get_attribute_descriptions() -> std::array<VkVertexInputAttributeDescription, 2> {
		std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions = {};

		attribute_descriptions[0].binding = 0;
		attribute_descriptions[0].location = 0;
		attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_descriptions[0].offset = offsetof(VVertex, pos);

		attribute_descriptions[1].binding = 0;
		attribute_descriptions[1].location = 1;
		attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[1].offset = offsetof(VVertex, color);

		return attribute_descriptions;
	}
};
struct VulkanSwapchain {
private:
public:
public:
	VkSwapchainKHR m_swapchain;

	//std::vector<VkImage> m_swapchain_images;
	//VkFormat m_swapchain_image_format;
	//VkExtent2D m_swapchain_extent;
};

struct VulkanInstance;
struct VulkanLogicalDevice {
private:
public:
	auto draw_frame() -> void;
	auto init(const VulkanInstance& instance) -> void;
	auto deinit() -> void;
public:
	const VulkanInstance* m_instance_p;
	VkDevice m_handle;
	const VulkanPhysicalDevice* m_physical_device_p;
	VkQueue m_graphics_queue;
	VkQueue m_present_queue;

public: // Swapchain stuff
	auto create_swapchain(const VulkanInstance& instance) -> void;
	auto recreate_swapchain() -> void;
	auto cleanup_swapchain() -> void;

	VkSwapchainKHR m_swapchain;
	std::vector<VkImage> m_swapchain_images;
	VkFormat m_swapchain_image_format;
	VkExtent2D m_swapchain_extent;

public:
	auto create_image_views() -> void;

	std::vector<VkImageView> m_swapchain_image_views;

public:
	auto create_render_pass() -> void;

	VkRenderPass m_render_pass;

public:
	auto create_graphics_pipeline() -> void;

	VkPipelineLayout m_pipeline_layout;
	VkPipeline m_graphics_pipeline;
public:
	auto create_framebuffers() -> void;

	std::vector<VkFramebuffer> m_framebuffers;

public:
	auto create_command_pool(const VulkanPhysicalDevice& physical_device) -> void;

	VkCommandPool m_command_pool;

public:
	auto create_command_buffers() -> void;

	std::vector<VkCommandBuffer> m_command_buffers;

public: // buffer stuff
	auto create_vertex_buffer(const std::vector<VVertex>& vertices) -> void;
	auto create_index_buffer(const std::vector<u16>& indices) -> void;
	auto create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) -> void;
	auto copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void;
	VkBuffer m_vertex_buffer;
	VkDeviceMemory m_vertex_buffer_memory;
	VkBuffer m_index_buffer;
	VkDeviceMemory m_index_buffer_memory;
public: // synchro objects
	auto create_sync_objects() -> void;

	std::vector<VkSemaphore> m_image_available_semaphores;
	std::vector<VkSemaphore> m_render_finished_semaphores;
	std::vector<VkFence> m_in_flight_fences;
	std::vector<VkFence> m_images_in_flight;
	size_t m_current_frame = 0;
	bool m_framebuffer_resized = false;
};