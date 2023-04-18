#include "pch.h"
#include "vulkan_pipeline.h"
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


namespace JadeFrame {
namespace vulkan {


static auto create_shader_module_from_spirv(const LogicalDevice& device, const std::vector<u32>& spirv)
    -> VkShaderModule {
    VkResult                       result;
    const VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = spirv.size() * 4, // NOTE: It has to be multiplied by 4!
        .pCode = spirv.data(),
    };

    VkShaderModule shader_module;
    result = vkCreateShaderModule(device.m_handle, &create_info, nullptr, &shader_module);
    if (result != VK_SUCCESS) assert(false);
    return shader_module;
}


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


static auto extract_descriptor_set_layouts(const LogicalDevice& device, const ReflectedCode& code)
    -> std::array<DescriptorSetLayout, static_cast<u8>(FREQUENCY::MAX)> {

    // TODO: Needs more checking.
    constexpr u8 max_sets = static_cast<u8>(FREQUENCY::MAX);

    std::array<DescriptorSetLayout, max_sets>                       set_layouts;
    std::array<std::vector<DescriptorSetLayout::Binding>, max_sets> bindings_set;

    for (const auto& module : code.m_modules) {
        const auto stage = from_SHADER_STAGE(module.m_stage);

        for (const auto& buffer : module.m_uniform_buffers) {
            auto type = get_uniform_buffer_type(static_cast<FREQUENCY>(buffer.set));
            bindings_set[buffer.set].emplace_back(buffer.binding, type, 1, stage);
        }
        for (const auto& image : module.m_sampled_images) {
            auto type = get_sampled_image_type(static_cast<FREQUENCY>(image.set));
            bindings_set[image.set].emplace_back(image.binding, type, 1, stage);
        }
    }
    for (u32 i = 0; i < set_layouts.size(); i++) {
        set_layouts[i] = device.create_descriptor_set_layout(bindings_set[i]);
    }
    return set_layouts;
}



static auto check_compatiblity(
    const std::vector<ReflectedCode::Module>& modules, const VkVertexInputBindingDescription& input_bindings,
    const std::vector<VkVertexInputAttributeDescription>& input_attributes) -> bool {
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
        for (u32 i = 0; i < vertex_module->m_inputs.size(); i++) { stride += vertex_module->m_inputs[i].size; }
        if (input_bindings.stride != stride) { compatible = false; }

        if (input_attributes.size() == vertex_module->m_inputs.size()) {
            for (u32 i = 0; i < input_attributes.size(); i++) {
                if (input_attributes[i].format != SHADER_TYPE_to_VkFormat(vertex_module->m_inputs[i].type)) {
                    compatible = false;
                }
                if (input_attributes[i].location != vertex_module->m_inputs[i].location) { compatible = false; }
            }
        } else {
            auto s0 = input_attributes.size();
            auto s1 = vertex_module->m_inputs.size();
            Logger::err(
                "Vertex input attributes count mismatch. `input_attributes.size()`: {} vs "
                "`vertex_module->m_inputs.size()`: {}",
                s0, s1);
            assert(false);
        }

    } else {
        compatible = false;
    }

    return compatible;
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
    other.m_layout = VK_NULL_HANDLE;
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
        other.m_layout = VK_NULL_HANDLE;
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
        vkDestroyPipeline(m_device->m_handle, m_handle, nullptr);
        vkDestroyPipelineLayout(m_device->m_handle, m_layout, nullptr);
    }
}

Pipeline::Pipeline(
    const LogicalDevice& device, const VkExtent2D& extent, const RenderPass& render_pass, const ShadingCode& code,
    const VertexFormat& vertex_format) {
    m_device = &device;
    m_render_pass = &render_pass;

    m_code.m_modules.resize(code.m_modules.size());
    for (u32 i = 0; i < m_code.m_modules.size(); i++) {
        m_code.m_modules[i].m_stage = code.m_modules[i].m_stage;
        m_code.m_modules[i].m_code = code.m_modules[i].m_code;
    }

    const ReflectedCode reflected_code = reflect(m_code);
    m_reflected_code = reflected_code;

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
    std::array<DescriptorSetLayout, 4> set_layouts = extract_descriptor_set_layouts(device, reflected_code);
    m_set_layouts = std::move(set_layouts);


    const VkVertexInputBindingDescription                binding_description = get_binding_description(vertex_format);
    const std::vector<VkVertexInputAttributeDescription> attribute_descriptions =
        get_attribute_descriptions(vertex_format);

    bool compatible = check_compatiblity(reflected_code.m_modules, binding_description, attribute_descriptions);
    JF_ASSERT(compatible == true, "The vertex format is not compatible with the vertex shader");

    std::vector<VkPushConstantRange> push_constant_ranges(m_push_constant_ranges.size());
    for (u32 i = 0; i < m_push_constant_ranges.size(); i++) {
        const PushConstantRange& range = m_push_constant_ranges[i];
        VkPushConstantRange&     vulkan_range = push_constant_ranges[i];

        vulkan_range.stageFlags = range.shader_stage;
        vulkan_range.offset = range.offset;
        vulkan_range.size = range.size;
    }
    VkDescriptorSetLayout set_layout_handles[4];
    for (u32 i = 0; i < m_set_layouts.size(); i++) { set_layout_handles[i] = m_set_layouts[i].m_handle; }
    const VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = static_cast<u32>(m_set_layouts.size()),
        .pSetLayouts = set_layout_handles,
        .pushConstantRangeCount = static_cast<u32>(push_constant_ranges.size()),
        .pPushConstantRanges = push_constant_ranges.data(),
    };
    VkResult result = vkCreatePipelineLayout(device.m_handle, &pipeline_layout_info, nullptr, &m_layout);
    if (result != VK_SUCCESS) assert(false);

    // Logging the pipeline layout
    {
        std::string offset = "  ";
        Logger::info("Created Pipeline Layout {} at {}", fmt::ptr(this), fmt::ptr(m_layout));
        Logger::info("\tSet Layouts count: {}", m_set_layouts.size());
        for (u32 i = 0; i < m_set_layouts.size(); i++) {
            std::string layout_str;
            switch (i) {
                case 0: layout_str = "Global"; break;
                case 1: layout_str = "Per-Pass"; break;
                case 2: layout_str = "Per-Material"; break;
                case 3: layout_str = "Per-Object"; break;
                default: layout_str = "Unknown"; break;
            }
            Logger::info("\t-Set layout: {} {}", i, layout_str);
            const DescriptorSetLayout& set_layout = m_set_layouts[i];
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
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    shader_stages.resize(m_code.m_modules.size());
    for (u32 i = 0; i < shader_stages.size(); i++) {
        shader_stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[i].pNext = nullptr;
        shader_stages[i].flags = 0;
        shader_stages[i].stage = from_SHADER_STAGE(m_code.m_modules[i].m_stage);
        shader_stages[i].module = create_shader_module_from_spirv(device, m_code.m_modules[i].m_code);
        shader_stages[i].pName = "main";
        shader_stages[i].pSpecializationInfo = nullptr;
    }

    const VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_description,
        .vertexAttributeDescriptionCount = static_cast<u32>(attribute_descriptions.size()),
        .pVertexAttributeDescriptions = attribute_descriptions.data(),
    };

    const VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    // NOTE: For OpenGL compatibility. make .height *= -1 and .y += .height
    constexpr bool   gl_compat = true;
    const VkViewport viewport = {
        .x = 0.0f,
        .y = gl_compat ? static_cast<f32>(extent.height) : 0.0f,
        .width = static_cast<f32>(extent.width),
        .height = gl_compat ? -static_cast<f32>(extent.height) : static_cast<f32>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    const VkRect2D scissor = {
        .offset = {0, 0},
        .extent = extent,
    };

    const VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    const VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE, // VK_CULL_MODE_BACK_BIT, //NOTE: after debugging make it cull mode back again
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = {},
        .depthBiasClamp = {},
        .depthBiasSlopeFactor = {},
        .lineWidth = 1.0f,
    };

    const VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = 0,
        .alphaToOneEnable = 0,
    };

    const VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = {},
        .dstColorBlendFactor = {},
        .colorBlendOp = {},
        .srcAlphaBlendFactor = {},
        .dstAlphaBlendFactor = {},
        .alphaBlendOp = {},
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

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
        .layout = m_layout,
        .renderPass = render_pass.m_handle,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    result = vkCreateGraphicsPipelines(device.m_handle, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_handle);
    if (result != VK_SUCCESS) assert(false);
    // Logger part
    {
        Logger::info("Created graphics pipeline {} at {}", fmt::ptr(this), fmt::ptr(m_handle));
        Logger::info("\tShader stages:");
        for (const auto& stage : shader_stages) {
            // Logger::info("\t\t{}", stage.module);
        }
    }

    for (u32 i = 0; i < shader_stages.size(); i++) {
        vkDestroyShaderModule(device.m_handle, shader_stages[i].module, nullptr);
    }

    m_device = &device;
}


} // namespace vulkan
} // namespace JadeFrame