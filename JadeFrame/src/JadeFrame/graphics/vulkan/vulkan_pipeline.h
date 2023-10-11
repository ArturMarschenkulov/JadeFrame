#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_descriptor_set.h"
#include "JadeFrame/prelude.h"
// #include "../graphics_shared.h"
#include "JadeFrame/graphics/reflect.h"

#include <thread>
#include <future>
#include <vector>
#include <JadeFrame/graphics/shader_loader.h>

namespace JadeFrame {

namespace vulkan {

class LogicalDevice;
class DescriptorSetLayout;

class RenderPass;

class Pipeline {
public:
    Pipeline() = default;
    ~Pipeline();
    Pipeline(const Pipeline&) = delete;
    auto operator=(const Pipeline& pipeline) = delete;
    Pipeline(Pipeline&& other) noexcept;
    auto operator=(Pipeline&& other) noexcept -> Pipeline&;

public:
    Pipeline(
        const LogicalDevice& device,
        const VkExtent2D&    extend,
        const RenderPass&    render_pass,
        const ShadingCode&   code,
        const VertexFormat&  vertex_format
    );

public:
    struct PushConstantRange {
        VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        u32                   offset = 0;
        u32                   size = 0;
    };

    class PipelineLayout {
    public:
        PipelineLayout() = default;
        ~PipelineLayout();
        PipelineLayout(const PipelineLayout& other) = delete;
        auto operator=(const PipelineLayout& other) -> PipelineLayout& = delete;
        PipelineLayout(PipelineLayout&& other) noexcept;
        auto operator=(PipelineLayout&& other) noexcept -> PipelineLayout&;

        PipelineLayout(
            const LogicalDevice& device,
            const std::array<DescriptorSetLayout, static_cast<u8>(FREQUENCY::MAX)>&
                                                  set_layouts,
            const std::vector<PushConstantRange>& push_constant_ranges
        );

        VkPipelineLayout     m_handle = VK_NULL_HANDLE;
        const LogicalDevice* m_device = nullptr;
    };

    VkPipeline           m_handle;
    PipelineLayout       m_layout;
    const LogicalDevice* m_device = nullptr;
    const RenderPass*    m_render_pass = nullptr;

    std::array<DescriptorSetLayout, static_cast<u8>(FREQUENCY::MAX)> m_set_layouts;
    ShadingCode                                                      m_code;

    bool m_is_compiled = false;

    // Reflect

    std::vector<PushConstantRange> m_push_constant_ranges;
    ReflectedCode                  m_reflected_code;
};

} // namespace vulkan
} // namespace JadeFrame