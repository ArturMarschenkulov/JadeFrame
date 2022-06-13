#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_descriptor_set.h"
#include "JadeFrame/prelude.h"
#include "../graphics_shared.h"

#include <thread>
#include <future>
#include <vector>
#include <JadeFrame/graphics/opengl/opengl_shader_loader.h>

namespace JadeFrame {
class VulkanLogicalDevice;
class VulkanDescriptorSetLayout;
class VulkanRenderPass;


class VulkanPipeline {
public:
    auto init(
        const VulkanLogicalDevice& device, const VkExtent2D& extend, const VulkanDescriptorSetLayout& descriptor_layout,
        const VulkanRenderPass& render_pass, const ShadingCode& code, const VertexFormat& vertex_format) -> void;
    auto deinit() -> void;

    auto operator=(const VulkanPipeline& pipeline);

public:
    VkPipeline                       m_handle;
    VkPipelineLayout                 m_layout;
    const VulkanLogicalDevice*       m_device = nullptr;
    const VulkanRenderPass*          m_render_pass = nullptr;
    const VulkanDescriptorSetLayout* m_descriptor_set_layout = nullptr;


    std::array<VulkanDescriptorSetLayout, static_cast<u8>(DESCRIPTOR_SET_FREQUENCY::MAX)> m_set_layouts;
    ShadingCode                                                                           m_code;

    bool m_is_compiled = false;

    // Reflect
    struct PushConstantRange {
        VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        u32                   offset = 0;
        u32                   size = 0;
    };
    std::vector<PushConstantRange> m_push_constant_ranges;
};
} // namespace JadeFrame