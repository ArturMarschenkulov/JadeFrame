#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include "JadeFrame/defines.h"


namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanCommandPool;
class VulkanSwapchain;
class VulkanRenderPass;
class VulkanDescriptorSets;
class VulkanBuffer;
class VulkanPipeline;

class VulkanCommandBuffers {
public:
	auto init(
		const VulkanLogicalDevice& device,
		const VulkanCommandPool& command_pool,
		const size_t amount = 1
	) -> void;
	auto deinit() -> void;

	auto record(size_t index, std::function<void()> func) -> void;
	auto draw_into(
		const VulkanRenderPass& render_pass,
		const VulkanSwapchain& swapchain,
		const VulkanPipeline& pipeline,
		const VulkanDescriptorSets& descriptor_sets,
		const VulkanBuffer& vertex_buffer,
		const VulkanBuffer& index_buffer,
		const std::vector<u16>& indices,
		const VkClearValue color_value
	) -> void;


	//auto bind_pipeline(size_t index) -> void;

public:
	std::vector<VkCommandBuffer> m_handles;
	const VulkanLogicalDevice* m_device = nullptr;
	const VulkanCommandPool* m_command_pool = nullptr;
};
}