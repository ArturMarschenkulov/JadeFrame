#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/defines.h"

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
		const GLSLCode& code
	) -> void;
	auto deinit() -> void;

	auto operator=(const VulkanPipeline& pipeline);
public:
	VkPipelineLayout m_pipeline_layout;
	VkPipeline m_graphics_pipeline;
	const VulkanLogicalDevice* m_device = nullptr;

	//VkShaderModule m_vert_shader_module = VK_NULL_HANDLE;
	//VkShaderModule m_frag_shader_module = VK_NULL_HANDLE;

	//std::future<std::vector<u32>> m_vert_shader_spirv;
	//std::future<std::vector<u32>> m_frag_shader_spirv;

	std::vector<u32> m_vert_shader_spirv;
	std::vector<u32> m_frag_shader_spirv;

	bool m_is_compiled = false;
};
}