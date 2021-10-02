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

	auto present_frame(const Matrix4x4& view_projection) -> void;
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
	std::vector<VulkanDescriptorSet> m_descriptor_sets;

public:
	auto update_uniform_buffer(VulkanBuffer& uniform_buffer, const Matrix4x4& view_projection) -> void;
	auto update_ubo(const Matrix4x4& view_projection) -> UniformBufferObject;
	std::vector<VulkanBuffer> m_uniform_buffers = { VULKAN_BUFFER_TYPE::UNIFORM };

public:
	VulkanCommandPool m_command_pool;
	std::vector<VulkanCommandBuffer> m_command_buffers;


public: // synchro objects
	std::vector<VulkanSemaphore> m_image_available_semaphores;
	std::vector<VulkanSemaphore> m_render_finished_semaphores;
	std::vector<VulkanFence> m_in_flight_fences;
	std::vector<VulkanFence> m_images_in_flight;

public: // Misc
	u32 m_present_image_index = 0;
	size_t m_current_frame = 0;
	bool m_framebuffer_resized = false;




//	// To be removed
//public:
//	VulkanPipeline m_pipeline;
//public: // buffer stuff
//	VulkanBuffer m_vertex_buffer = { VULKAN_BUFFER_TYPE::VERTEX };
//	VulkanBuffer m_index_buffer = { VULKAN_BUFFER_TYPE::INDEX };
//
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