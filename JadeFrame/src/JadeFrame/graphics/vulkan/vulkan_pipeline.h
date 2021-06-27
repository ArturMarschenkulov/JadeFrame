#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {
class VulkanLogicalDevice;

class VulkanPipeline {
public:
	auto init(const VulkanLogicalDevice& device) -> void;
	auto deinit() -> void;

	VkPipelineLayout m_pipeline_layout;
	VkPipeline m_graphics_pipeline;
};
}