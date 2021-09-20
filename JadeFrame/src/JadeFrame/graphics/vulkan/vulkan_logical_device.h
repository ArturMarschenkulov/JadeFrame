#pragma once

#include <vulkan/vulkan.h>
#include "vulkan_shared.h"
#include "vulkan_swapchain.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_command_pool.h"
#include "vulkan_buffer.h"
#include "vulkan_descriptor_set_layout.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_sync_object.h"
#include "vulkan_command_buffers.h"
#include "vulkan_queue.h"

#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/vec_3.h"

#include "JadeFrame/defines.h"

#include <array>
#include <vector>

namespace JadeFrame {
class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanPipeline;
class VulkanBuffer;

class VulkanLogicalDevice {
private:
public:
	auto init(const VulkanInstance& instance, const VulkanPhysicalDevice& physical_device) -> void;
	auto deinit() -> void;

	auto draw_frame(const Matrix4x4& view_projection) -> void;
public:
	const VulkanInstance* m_instance_p = nullptr;
	VkDevice m_handle = nullptr;
	const VulkanPhysicalDevice* m_physical_device_p = nullptr;
	VulkanQueue m_graphics_queue;
	VulkanQueue m_present_queue;

public: // Swapchain stuff
	auto recreate_swapchain() -> void;
	auto cleanup_swapchain() -> void;

	VulkanSwapchain m_swapchain;

public:
	VulkanRenderPass m_render_pass;

public: // Descriptor set
	VulkanDescriptorSetLayout m_descriptor_set_layout;
	VulkanDescriptorPool m_descriptor_pool;

	//auto create_descriptor_sets(u32 image_amount) -> void;
	//std::vector<VkDescriptorSet> m_descriptor_sets;
	VulkanDescriptorSets m_descriptor_sets;

	auto update_uniform_buffer(u32 current_image, const Matrix4x4& view_projection) -> void;
public: // synchro objects
	auto create_sync_objects() -> void;
	std::vector<VulkanSemaphore> m_image_available_semaphores;
	std::vector<VulkanSemaphore> m_render_finished_semaphores;
	std::vector<VulkanFence> m_in_flight_fences;
	std::vector<VulkanFence> m_images_in_flight;
	size_t m_current_frame = 0;
	bool m_framebuffer_resized = false;

public:
	VulkanCommandPool m_command_pool;

public:
	auto draw_into_command_buffers(
		const VulkanRenderPass& render_pass,
		const VulkanSwapchain& swapchain,
		const VulkanPipeline& pipeline,
		const VulkanDescriptorSets& descriptor_sets,
		const VulkanBuffer& vertex_buffer,
		const VulkanBuffer& index_buffer,
		const std::vector<u16>& indices,
		const VkClearValue color_value
	) -> void;

	VulkanCommandBuffers m_command_buffers;


public:
	VulkanPipeline m_pipeline;
public: // buffer stuff
	VulkanBuffer m_vertex_buffer = { VULKAN_BUFFER_TYPE::VERTEX };
	VulkanBuffer m_index_buffer = { VULKAN_BUFFER_TYPE::INDEX };
	std::vector<VulkanBuffer> m_uniform_buffers = { VULKAN_BUFFER_TYPE::UNIFORM };

public: // texture stuff
	auto create_texture_image(const std::string& path) -> void;
	auto create_image(u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) -> void;
	auto transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) -> void;
	auto copy_buffer_to_image(VkBuffer buffer, VkImage image, u32 width, u32 height) -> void;
	auto begin_single_time_commands()->VkCommandBuffer;
	auto end_single_time_commands(VkCommandBuffer command_buffer) -> void;
	VkImage m_texture_image;
	VkDeviceMemory m_texture_image_Memory;

	auto create_image_view(VkImage image, VkFormat format)->VkImageView;
	auto create_texture_image_view() -> void;
	auto create_texture_sampler() -> void;

	VkImageView m_texture_image_view;
	VkSampler m_texture_sampler;
};
}