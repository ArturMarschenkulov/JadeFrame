#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/defines.h"

#include <vector>


namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanCommandPool;
class VulkanSwapchain;
class VulkanRenderPass;
class VulkanDescriptorSet;
class VulkanBuffer;
class VulkanPipeline;
class VulkanFramebuffer;
class QueueFamilyIndices;

class VulkanCommandBuffer {
public:
	auto record_begin() -> void;
	auto record_end() -> void;
	auto render_pass_begin(const VulkanFramebuffer& framebuffer, const VulkanRenderPass& render_pass, const VulkanSwapchain& swapchain, VkClearValue color) -> void;
	auto render_pass_end() -> void;

	auto draw_into(
		size_t index,
		const VulkanRenderPass& render_pass,
		const VulkanSwapchain& swapchain,
		const VulkanPipeline& pipeline,
		const std::vector<VulkanDescriptorSet>& descriptor_sets,
		const Vulkan_GPUMeshData& gpu_data,
		const VertexData& vertex_data,
		const VkClearValue color_value
	) -> void;

	auto reset() -> void;

public:
	VkCommandBuffer m_handle;
	const VulkanLogicalDevice* m_device = nullptr;
	const VulkanCommandPool* m_command_pool = nullptr;
};

using QueueFamilyIndex = u32;
class VulkanCommandPool {
public:
	auto init(const VulkanLogicalDevice& device, const QueueFamilyIndex& queue_family_index) -> void;
	auto deinit() -> void;

	auto allocate_command_buffers(u32 amount) const->std::vector<VulkanCommandBuffer>;
	auto allocate_command_buffer() const->VulkanCommandBuffer;
	auto free_command_buffers(const std::vector<VulkanCommandBuffer>& command_buffers) const -> void;
public:
	const VulkanLogicalDevice* m_device = nullptr;
	VkCommandPool m_handle = VK_NULL_HANDLE;
};
}