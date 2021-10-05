#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/defines.h"
#include "../graphics_shared.h"

#include <thread>
#include <future>
#include <vector>
#include <JadeFrame/graphics/opengl/opengl_shader_loader.h>

namespace JadeFrame {
class VulkanLogicalDevice;
class VulkanSwapchain;
class VulkanDescriptorSetLayout;
class VulkanRenderPass;


class VulkanPipeline {
public:
	auto init(
		const VulkanLogicalDevice& device, 
		const VkExtent2D& extend,
		const VulkanDescriptorSetLayout& descriptor_layout, 
		const VulkanRenderPass& render_pass,
		const ShadingCode& code,
		const VertexFormat& vertex_format
	) -> void;
	auto deinit() -> void;

	auto operator=(const VulkanPipeline& pipeline);
public:
	VkPipelineLayout m_pipeline_layout;
	VkPipeline m_graphics_pipeline;
	const VulkanLogicalDevice* m_device = nullptr;

	std::vector<u32> m_vert_shader_spirv;
	std::vector<u32> m_frag_shader_spirv;

	bool m_is_compiled = false;

	// Reflect
	struct PushConstantRange {
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		u32 offset = 0;
		u32 size = 0;
	};
	std::vector<PushConstantRange> m_push_constant_ranges;
};
}