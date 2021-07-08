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

#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/vec_3.h"

#include <array>
#include <vector>

namespace JadeFrame {
class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanPipeline;

class VulkanLogicalDevice {
private:
public:
	auto init(const VulkanInstance& instance) -> void;
	auto deinit() -> void;

	auto draw_frame() -> void;
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
	VulkanDescriptorSetLayout m_descriptor_set_layout;
	VulkanDescriptorPool m_descriptor_pool;

	//auto create_descriptor_sets(u32 image_amount) -> void;
	//std::vector<VkDescriptorSet> m_descriptor_sets;
	VulkanDescriptorSets m_descriptor_sets;

	auto update_uniform_buffer(u32 current_image) -> void;


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
}