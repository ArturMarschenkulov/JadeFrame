#pragma once
#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/vec_3.h"

#include "vulkan_shared.h"
#include "vulkan_swapchain.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_command_pool.h"
#include "vulkan_buffer.h"

#include <array>
#include <vector>

class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanPipeline;

class VulkanLogicalDevice {
private:
public:
	auto draw_frame() -> void;
	auto init(const VulkanInstance& instance) -> void;
	auto deinit() -> void;
public:
	const VulkanInstance* m_instance_p = nullptr;
	VkDevice m_handle = nullptr;
	const VulkanPhysicalDevice* m_physical_device_p = nullptr;
	VkQueue m_graphics_queue = nullptr;
	VkQueue m_present_queue = nullptr;

public: // Swapchain stuff
	auto recreate_swapchain() -> void;
	auto cleanup_swapchain() -> void;

	VulkanSwapchain m_swapchain;

public:
	VulkanRenderPass m_render_pass;

public: // Descriptor set
	auto create_descriptor_set_layout() -> void;
	auto update_uniform_buffer(u32 current_image) -> void;
	auto create_descriptor_pool() -> void;
	auto create_descriptor_sets() -> void;
	VkDescriptorPool m_descriptor_pool;
	VkDescriptorSetLayout m_descriptor_set_layout;
	std::vector<VkDescriptorSet> m_descriptor_sets;
public:
	VulkanPipeline m_pipeline;

public:
	VulkanCommandPool m_command_pool;

public:
	auto create_command_buffers() -> void;

	std::vector<VkCommandBuffer> m_command_buffers;

public: // buffer stuff
	VulkanBuffer m_vertex_buffer = { VULKAN_BUFFER_TYPE::VERTEX };
	VulkanBuffer m_index_buffer = { VULKAN_BUFFER_TYPE::INDEX };
	std::vector<VulkanBuffer> m_uniform_buffers = { VULKAN_BUFFER_TYPE::UNIFORM };

public: // synchro objects
	auto create_sync_objects() -> void;

	std::vector<VkSemaphore> m_image_available_semaphores;
	std::vector<VkSemaphore> m_render_finished_semaphores;
	std::vector<VkFence> m_in_flight_fences;
	std::vector<VkFence> m_images_in_flight;
	size_t m_current_frame = 0;
	bool m_framebuffer_resized = false;
};