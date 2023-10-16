#include "pch.h"
#include "vulkan_pipeline.h"
#include "vulkan_context.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_shared.h"

#include "SPIRV-Cross/spirv_glsl.hpp"
#include "SPIRV-Cross/spirv_hlsl.hpp"
#include "SPIRV-Cross/spirv_msl.hpp"

#include "JadeFrame/utils/assert.h"

#include <array>
#include <span>

namespace JadeFrame {
namespace vulkan {

static auto from_SHADER_STAGE(SHADER_STAGE stage) -> VkShaderStageFlagBits {
    VkShaderStageFlagBits result = {};
    switch (stage) {
        case SHADER_STAGE::VERTEX: {
            result = VK_SHADER_STAGE_VERTEX_BIT;
        } break;
        case SHADER_STAGE::FRAGMENT: {
            result = VK_SHADER_STAGE_FRAGMENT_BIT;
        } break;
        default: assert(false);
    }
    return result;
}

static auto check_compatiblity(
    const std::vector<ReflectedCode::Module>&             modules,
    const VkVertexInputBindingDescription&                input_bindings,
    const std::vector<VkVertexInputAttributeDescription>& input_attributes
) -> bool {
    bool compatible = true;

    const ReflectedCode::Module* vertex_module = nullptr;
    for (u32 i = 0; i < modules.size(); i++) {
        if (modules[i].m_stage == SHADER_STAGE::VERTEX) {
            vertex_module = &modules[i];
            break;
        }
    }

    if (vertex_module != nullptr) {
        u32 stride = 0;
        for (u32 i = 0; i < vertex_module->m_inputs.size(); i++) {
            stride += vertex_module->m_inputs[i].size;
        }
        if (input_bindings.stride != stride) { compatible = false; }

        if (input_attributes.size() == vertex_module->m_inputs.size()) {
            for (u32 i = 0; i < input_attributes.size(); i++) {
                if (input_attributes[i].format !=
                    SHADER_TYPE_to_VkFormat(vertex_module->m_inputs[i].type)) {
                    compatible = false;
                }
                if (input_attributes[i].location != vertex_module->m_inputs[i].location) {
                    compatible = false;
                }
            }
        } else {
            auto s0 = input_attributes.size();
            auto s1 = vertex_module->m_inputs.size();
            Logger::err(
                "Vertex input attributes count mismatch. `input_attributes.size()`: {} "
                "vs "
                "`vertex_module->m_inputs.size()`: {}",
                s0,
                s1
            );
            assert(false);
        }

    } else {
        compatible = false;
    }

    return compatible;
}

/*--------------------
    ShaderModule
----------------------*/
class ShaderModule {
public:
    ShaderModule() = default;
    ~ShaderModule();
    ShaderModule(const ShaderModule&) = delete;
    auto operator=(const ShaderModule&) -> ShaderModule& = delete;
    ShaderModule(ShaderModule&& other) noexcept;
    auto operator=(ShaderModule&& other) noexcept -> ShaderModule&;

    ShaderModule(
        const LogicalDevice&    device,
        const std::vector<u32>& spirv,
        SHADER_STAGE            stage
    );

public:
    const LogicalDevice* m_device = nullptr;
    VkShaderModule       m_handle = VK_NULL_HANDLE;
    SHADER_STAGE         m_stage = SHADER_STAGE::VERTEX;
    std::vector<u32>     m_spirv;
    ReflectedModule      m_reflected;
};

ShaderModule::~ShaderModule() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_device->m_handle, m_handle, Instance::allocator());
    }
}

ShaderModule::ShaderModule(ShaderModule&& other) noexcept
    : m_device(other.m_device)
    , m_handle(other.m_handle)
    , m_stage(other.m_stage)
    , m_spirv(std::move(other.m_spirv)) {
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = VK_NULL_HANDLE;
}

auto ShaderModule::operator=(ShaderModule&& other) noexcept -> ShaderModule& {
    if (this != &other) {
        m_device = other.m_device;
        m_handle = other.m_handle;
        m_stage = other.m_stage;
        m_spirv = std::move(other.m_spirv);
        m_reflected = std::move(other.m_reflected);
        other.m_handle = VK_NULL_HANDLE;
    }
    return *this;
}

ShaderModule::ShaderModule(
    const LogicalDevice&    device,
    const std::vector<u32>& spirv,
    SHADER_STAGE            stage
)
    : m_device(&device)
    , m_stage(stage)
    , m_spirv(spirv)
    , m_reflected(reflect(spirv)) {

    const VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = spirv.size() * 4, // NOTE: It has to be multiplied by 4!
        .pCode = spirv.data(),
    };

    VkResult result = vkCreateShaderModule(
        m_device->m_handle, &create_info, Instance::allocator(), &m_handle
    );
    if (result != VK_SUCCESS) {
        Logger::err("Failed to create shader module.");
        assert(false);
    }
}

static auto get_uniform_buffer_type(FREQUENCY freq) -> VkDescriptorType {
    VkDescriptorType result = {};
    switch (freq) {
        case FREQUENCY::PER_OBJECT: {
            result = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        } break;
        case FREQUENCY::PER_FRAME: {
            result = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        } break;
        default: assert(false);
    }
    return result;
}

static auto get_sampled_image_type(FREQUENCY freq) -> VkDescriptorType {
    VkDescriptorType result = {};
    switch (freq) {
        case FREQUENCY::PER_OBJECT: {
            result = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        } break;
        case FREQUENCY::PER_FRAME: {
            result = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        } break;
        default: result = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
    return result;
}

static auto extract_descriptor_set_layouts(const std::span<ShaderModule>& modules)
    -> std::array<DescriptorSetLayout, static_cast<u8>(FREQUENCY::MAX)> {

    // TODO: Needs more checking.
    constexpr u8 max_sets = static_cast<u8>(FREQUENCY::MAX);

    std::array<DescriptorSetLayout, max_sets>                       set_layouts;
    std::array<std::vector<DescriptorSetLayout::Binding>, max_sets> bindings_set;

    for (const auto& module : modules) {
        const auto stage = from_SHADER_STAGE(module.m_stage);

        for (const auto& buffer : module.m_reflected.m_uniform_buffers) {
            auto type = get_uniform_buffer_type(static_cast<FREQUENCY>(buffer.set));
            bindings_set[buffer.set].emplace_back(buffer.binding, type, 1, stage);
        }
        for (const auto& image : module.m_reflected.m_sampled_images) {
            auto type = get_sampled_image_type(static_cast<FREQUENCY>(image.set));
            bindings_set[image.set].emplace_back(image.binding, type, 1, stage);
        }
    }
    auto& dev = modules[0].m_device;
    for (u32 i = 0; i < set_layouts.size(); i++) {
        set_layouts[i] = dev->create_descriptor_set_layout(bindings_set[i]);
    }
    return set_layouts;
}

/*--------------------
    Pipeline Layout
----------------------*/

static auto layout_create_info(
    const std::span<const VkDescriptorSetLayout>& layouts,
    const std::span<VkPushConstantRange>&         push_constants
) -> VkPipelineLayoutCreateInfo {

    const VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = static_cast<u32>(layouts.size()),
        .pSetLayouts = layouts.data(),
        .pushConstantRangeCount = static_cast<u32>(push_constants.size()),
        .pPushConstantRanges = push_constants.data(),
    };
    return layout_info;
}

Pipeline::PipelineLayout::~PipelineLayout() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device->m_handle, m_handle, Instance::allocator());
    }
}

Pipeline::PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept {
    m_handle = other.m_handle;
    m_device = other.m_device;
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
}

auto Pipeline::PipelineLayout::operator=(PipelineLayout&& other) noexcept
    -> PipelineLayout& {
    if (this != &other) {
        m_handle = other.m_handle;
        m_device = other.m_device;
        other.m_handle = VK_NULL_HANDLE;
        other.m_device = nullptr;
    }
    return *this;
}

Pipeline::PipelineLayout::PipelineLayout(
    const LogicalDevice&                                                    device,
    const std::array<DescriptorSetLayout, static_cast<u8>(FREQUENCY::MAX)>& set_layouts,
    const std::vector<Pipeline::PushConstantRange>& push_constant_ranges
) {

    m_device = &device;

    std::vector<VkPushConstantRange> push_constants;
    push_constants.resize(push_constant_ranges.size());
    for (u32 i = 0; i < push_constant_ranges.size(); i++) {
        const Pipeline::PushConstantRange& range = push_constant_ranges[i];
        VkPushConstantRange&               vulkan_range = push_constants[i];

        vulkan_range.stageFlags = range.shader_stage;
        vulkan_range.offset = range.offset;
        vulkan_range.size = range.size;
    }

    std::array<VkDescriptorSetLayout, 4> set_layout_handles;
    for (u32 i = 0; i < set_layouts.size(); i++) {
        set_layout_handles[i] = set_layouts[i].m_handle;
    }

    const VkPipelineLayoutCreateInfo pipeline_layout_info =
        layout_create_info(set_layout_handles, push_constants);
    VkResult result = vkCreatePipelineLayout(
        device.m_handle, &pipeline_layout_info, Instance::allocator(), &m_handle
    );
    if (result != VK_SUCCESS) { assert(false); }

#if 0
    // Logging the pipeline layout
    {
        std::string offset = "  ";
        Logger::info("Created Pipeline Layout {} at {}", fmt::ptr(this), fmt::ptr(m_handle));
        Logger::info("\tSet Layouts count: {}", set_layouts.size());
        for (u32 i = 0; i < set_layouts.size(); i++) {
            std::string layout_str;
            switch (i) {
                case 0: layout_str = "Global"; break;
                case 1: layout_str = "Per-Pass"; break;
                case 2: layout_str = "Per-Material"; break;
                case 3: layout_str = "Per-Object"; break;
                default: layout_str = "Unknown"; break;
            }
            Logger::info("\t-Set layout: {} {}", i, layout_str);
            const DescriptorSetLayout& set_layout = set_layouts[i];
            for (u32 j = 0; j < set_layout.m_bindings.size(); j++) {
                const VkDescriptorSetLayoutBinding& binding = set_layout.m_bindings[j];
                Logger::info("\t\tBinding: {}", binding.binding);
                Logger::info("\t\t-Type: {}", to_string(binding.descriptorType));
                Logger::info("\t\t-Count: {}", binding.descriptorCount);
                Logger::info("\t\t-Stage: {}", to_string_from_shader_stage_flags(binding.stageFlags));
            }
        }
        Logger::info("Push constant ranges count: {}", push_constant_ranges.size());
        for (u32 i = 0; i < push_constant_ranges.size(); i++) {
            const VkPushConstantRange& push_constant_range = push_constant_ranges[i];
            Logger::info("\tPush constant range: {}", i);
            Logger::info("\t\tStage: {}", to_string_from_shader_stage_flags(push_constant_range.stageFlags));
            Logger::info("\t\tOffset: {}", push_constant_range.offset);
            Logger::info("\t\tSize: {}", push_constant_range.size);
        }
    }
#endif
}

/*--------------------
    Pipeline
----------------------*/

static auto
shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shader_module)
    -> VkPipelineShaderStageCreateInfo {
    const VkPipelineShaderStageCreateInfo shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = stage,
        .module = shader_module,
        .pName = "main",
        .pSpecializationInfo = nullptr,
    };
    return shader_stage_info;
}

static auto
viewport_state_create_info(const VkViewport& viewport, const VkRect2D& scissor)
    -> VkPipelineViewportStateCreateInfo {
    const VkPipelineViewportStateCreateInfo viewport_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };
    return viewport_info;
}

static auto get_binding_description(const VertexFormat& vertex_format)
    -> VkVertexInputBindingDescription {
    u32 stride = 0;
    for (const VertexAttribute& attribute : vertex_format.m_attributes) {
        stride += attribute.size;
    }
    VkVertexInputBindingDescription const binding_description = {
        .binding = 0,
        .stride = stride,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    return binding_description;
}

static auto get_attribute_descriptions(const VertexFormat& vertex_format)
    -> std::vector<VkVertexInputAttributeDescription> {
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
    attribute_descriptions.resize(vertex_format.m_attributes.size());
    for (u32 i = 0; i < vertex_format.m_attributes.size(); i++) {
        attribute_descriptions[i].binding = 0;
        attribute_descriptions[i].location = i;
        attribute_descriptions[i].format =
            SHADER_TYPE_to_VkFormat(vertex_format.m_attributes[i].type);
        attribute_descriptions[i].offset =
            static_cast<u32>(vertex_format.m_attributes[i].offset);
    }

    return attribute_descriptions;
}

static auto vertex_input_state_create_info(
    VkVertexInputBindingDescription                    bindings,
    const std::span<VkVertexInputAttributeDescription> attributes
) -> VkPipelineVertexInputStateCreateInfo {

    const VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindings,
        .vertexAttributeDescriptionCount = static_cast<u32>(attributes.size()),
        .pVertexAttributeDescriptions = attributes.data(),
    };
    return vertex_input_info;
}

static auto input_assembly_state_create_info() -> VkPipelineInputAssemblyStateCreateInfo {
    const VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
    return input_assembly;
}

static auto rasterization_state_create_info() -> VkPipelineRasterizationStateCreateInfo {
    const VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE, // VK_CULL_MODE_BACK_BIT, //NOTE: after debugging
                                       // make it cull mode back again
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = {},
        .depthBiasClamp = {},
        .depthBiasSlopeFactor = {},
        .lineWidth = 1.0f,
    };
    return rasterizer;
}

static auto multisample_state_create_info() -> VkPipelineMultisampleStateCreateInfo {
    const VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };
    return multisampling;
}

static auto color_blend_attachment_state() -> VkPipelineColorBlendAttachmentState {
    // const VkPipelineColorBlendAttachmentState color_blend_attachment = {
    //     .blendEnable = VK_FALSE,
    //     .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    //     .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    //     .colorBlendOp = VK_BLEND_OP_ADD,
    //     .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    //     .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    //     .alphaBlendOp = VK_BLEND_OP_ADD,
    //     .colorWriteMask =
    //         VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    //         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    // };
    const VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = {},
        .dstColorBlendFactor = {},
        .colorBlendOp = {},
        .srcAlphaBlendFactor = {},
        .dstAlphaBlendFactor = {},
        .alphaBlendOp = {},
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    return color_blend_attachment;
}

static auto color_blend_state_create_info(
    const VkPipelineColorBlendAttachmentState& color_blend_attachment
) -> VkPipelineColorBlendStateCreateInfo {
    const VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };
    return color_blending;
}

Pipeline::Pipeline(Pipeline&& other) noexcept
    : m_handle(other.m_handle)
    , m_layout(std::move(other.m_layout))
    , m_device(other.m_device)
    , m_render_pass(other.m_render_pass)
    , m_set_layouts(std::move(other.m_set_layouts))
    , m_code(std::move(other.m_code))
    , m_is_compiled(other.m_is_compiled)
    , m_push_constant_ranges(other.m_push_constant_ranges)
    , m_reflected_code(other.m_reflected_code) {
    other.m_handle = VK_NULL_HANDLE;
    other.m_layout = {};
    other.m_device = nullptr;
    other.m_render_pass = nullptr;
    other.m_set_layouts = {};
    other.m_code = {};
    other.m_is_compiled = false;
    other.m_push_constant_ranges.clear();
    other.m_reflected_code = {};
}

auto Pipeline::operator=(Pipeline&& other) noexcept -> Pipeline& {
    if (this != &other) {
        m_handle = other.m_handle;
        m_layout = std::move(other.m_layout);
        m_device = other.m_device;
        m_render_pass = other.m_render_pass;
        m_set_layouts = std::move(other.m_set_layouts);
        m_code = std::move(other.m_code);
        m_is_compiled = other.m_is_compiled;
        m_push_constant_ranges = other.m_push_constant_ranges;
        m_reflected_code = other.m_reflected_code;

        other.m_handle = VK_NULL_HANDLE;
        other.m_layout = {};
        other.m_device = nullptr;
        other.m_render_pass = nullptr;
        other.m_set_layouts = {};
        other.m_code = {};
        other.m_is_compiled = false;
        other.m_push_constant_ranges.clear();
        other.m_reflected_code = {};
    }
    return *this;
}

Pipeline::~Pipeline() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device->m_handle, m_handle, Instance::allocator());
    }
}

Pipeline::Pipeline(
    const LogicalDevice& device,
    const VkExtent2D&    extent,
    const RenderPass&    render_pass,
    const ShadingCode&   code,
    const VertexFormat&  vertex_format
) {
    m_device = &device;
    m_render_pass = &render_pass;

    m_code.m_modules.resize(code.m_modules.size());
    std::vector<ShaderModule> shader_modules;
    shader_modules.resize(code.m_modules.size());
    for (u32 i = 0; i < shader_modules.size(); i++) {
        auto& code_ = code.m_modules[i].m_code;
        auto& stage_ = code.m_modules[i].m_stage;
        shader_modules[i] = ShaderModule(device, code_, stage_);
    }
    /*
        There are always 4 descriptor set layouts. They are grouped by binding frequency.
        0 - Used for engine global resources. Bound once per frame.
        1 - Used for per-pass resources. Bound once per pass.
        2 - Used for per-material resources. Bound once per material.
        3 - Used for per-object resources. Bound once per object.

        set 0: projection and view matrix. point lights.
        set 1:
        set 2: materials. textures. samplers.
        set 3: model matrix.
    */
    std::array<DescriptorSetLayout, 4> set_layouts =
        extract_descriptor_set_layouts(shader_modules);
    m_set_layouts = std::move(set_layouts);

    // bool compatible = check_compatiblity(reflected_code.m_modules, binding_description,
    // attribute_descriptions); JF_ASSERT(compatible == true, "The vertex format is not
    // compatible with the vertex shader");

    m_layout = PipelineLayout(device, m_set_layouts, m_push_constant_ranges);

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    shader_stages.resize(m_code.m_modules.size());
    for (u32 i = 0; i < shader_stages.size(); i++) {
        shader_stages[i] = shader_stage_create_info(
            from_SHADER_STAGE(shader_modules[i].m_stage), shader_modules[i].m_handle
        );
    }

    // NOTE: For OpenGL compatibility. make .height *= -1 and .y += .height
    constexpr bool   gl_compat = true;
    const VkViewport viewport = {
        .x = 0.0f,
        .y = gl_compat ? static_cast<f32>(extent.height) : 0.0f,
        .width = static_cast<f32>(extent.width),
        .height = gl_compat ? -static_cast<f32>(extent.height)
                            : static_cast<f32>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    const VkRect2D scissor = {
        .offset = {0, 0},
        .extent = extent,
    };

    const VkPipelineInputAssemblyStateCreateInfo input_assembly =
        input_assembly_state_create_info();
    const VkPipelineRasterizationStateCreateInfo rasterizer =
        rasterization_state_create_info();
    const VkPipelineMultisampleStateCreateInfo multisampling =
        multisample_state_create_info();
    const VkPipelineColorBlendAttachmentState color_blend_attachment =
        color_blend_attachment_state();
    const VkPipelineViewportStateCreateInfo viewport_state =
        viewport_state_create_info(viewport, scissor);
    const VkPipelineColorBlendStateCreateInfo color_blending =
        color_blend_state_create_info(color_blend_attachment);

    const VkVertexInputBindingDescription binding_description =
        get_binding_description(vertex_format);
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions =
        get_attribute_descriptions(vertex_format);
    const VkPipelineVertexInputStateCreateInfo vertex_input_info =
        vertex_input_state_create_info(binding_description, attribute_descriptions);

    const VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = static_cast<u32>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blending,
        .pDynamicState = nullptr,
        .layout = m_layout.m_handle,
        .renderPass = render_pass.m_handle,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    VkResult result = vkCreateGraphicsPipelines(
        device.m_handle,
        VK_NULL_HANDLE,
        1,
        &pipeline_info,
        Instance::allocator(),
        &m_handle
    );
    if (result != VK_SUCCESS) { assert(false); }
    // Logger part
    {
        Logger::info(
            "Created graphics pipeline {} at {}", fmt::ptr(this), fmt::ptr(m_handle)
        );
        Logger::info("\tShader stages:");
        for (const auto& stage : shader_stages) {
            // Logger::info("\t\t{}", stage.module);
        }
    }

    m_device = &device;
}

} // namespace vulkan
} // namespace JadeFrame