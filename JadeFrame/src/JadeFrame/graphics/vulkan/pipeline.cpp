#include "pipeline.h"
#include "JadeFrame/utils/logger.h"
#include "context.h"
#include "logical_device.h"
#include "physical_device.h"
#include "swapchain.h"
#include "descriptor_set.h"
#include "shared.h"

#include <algorithm>
#include <array>
#include <map>
#include <span>
#include <utility>

namespace JadeFrame {
namespace vulkan {

static auto to_ShaderStageFlagsBits(SHADER_STAGE stage) -> VkShaderStageFlagBits {
    VkShaderStageFlagBits result = {};
    switch (stage) {
        case SHADER_STAGE::VERTEX: {
            result = VK_SHADER_STAGE_VERTEX_BIT;
        } break;
        case SHADER_STAGE::FRAGMENT: {
            result = VK_SHADER_STAGE_FRAGMENT_BIT;
        } break;
        case SHADER_STAGE::GEOMETRY: {
            result = VK_SHADER_STAGE_GEOMETRY_BIT;
        } break;
        default: assert(false);
    }
    return result;
}

static auto to_vk_vertex_format(SHADER_TYPE type) -> VkFormat {
    switch (type) {
        case SHADER_TYPE::V_2_F32: return VK_FORMAT_R32G32_SFLOAT;
        case SHADER_TYPE::V_3_F32: return VK_FORMAT_R32G32B32_SFLOAT;
        case SHADER_TYPE::V_4_F32: return VK_FORMAT_R32G32B32A32_SFLOAT;
        default:
            Logger::err("Unsupported vertex shader input type {}", to_string(type));
            return VK_FORMAT_UNDEFINED;
    }
}

static auto check_compatibility(
    const std::vector<ReflectedModule>&                   modules,
    const VkVertexInputBindingDescription&                input_binding,
    const std::vector<VkVertexInputAttributeDescription>& input_attributes
) -> bool {
    bool compatible = true;

    const ReflectedModule* vertex_module = nullptr;
    const ReflectedModule* fragment_module = nullptr;
    for (const auto& module : modules) {
        if (module.m_stage == SHADER_STAGE::VERTEX) { vertex_module = &module; }
        if (module.m_stage == SHADER_STAGE::FRAGMENT) { fragment_module = &module; }
    }

    if (vertex_module == nullptr) {
        Logger::err("Graphics pipeline has no vertex shader module");
        return false;
    }
    if (fragment_module == nullptr) {
        Logger::err("Graphics pipeline has no fragment shader module");
        return false;
    }

    u32 reflected_stride = 0;
    for (const auto& input : vertex_module->m_inputs) {
        reflected_stride += input.size;
    }
    if (input_binding.stride != reflected_stride) {
        Logger::err(
            "Vertex stride mismatch: pipeline {}, shader {}",
            input_binding.stride,
            reflected_stride
        );
        compatible = false;
    }

    if (input_attributes.size() != vertex_module->m_inputs.size()) {
        Logger::err(
            "Vertex attribute count mismatch: pipeline {}, shader {}",
            input_attributes.size(),
            vertex_module->m_inputs.size()
        );
        compatible = false;
    }

    for (const auto& input : vertex_module->m_inputs) {
        const auto attribute = std::find_if(
            input_attributes.begin(),
            input_attributes.end(),
            [&](const VkVertexInputAttributeDescription& value) {
                return value.location == input.location;
            }
        );
        if (attribute == input_attributes.end()) {
            Logger::err("Vertex input location {} is missing from the vertex buffer", input.location);
            compatible = false;
            continue;
        }
        const VkFormat expected_format = to_vk_vertex_format(input.type);
        if (attribute->format != expected_format) {
            Logger::err("Vertex input location {} has an incompatible format", input.location);
            compatible = false;
        }
        if (input.size != get_size(input.type)) {
            Logger::err(
                "Vertex input location {} has an unsupported reflected size {}",
                input.location,
                input.size
            );
            compatible = false;
        }
    }

    // Validate the shader-to-shader interface as well. Vertex outputs which
    // are not consumed are legal, but every fragment input must be provided
    // by the vertex stage with the same location and type.
    for (const auto& input : fragment_module->m_inputs) {
        const auto output = std::find_if(
            vertex_module->m_outputs.begin(),
            vertex_module->m_outputs.end(),
            [&](const ReflectedModule::Output& value) {
                return value.location == input.location;
            }
        );
        if (output == vertex_module->m_outputs.end()) {
            Logger::err(
                "Fragment input location {} is not written by the vertex shader",
                input.location
            );
            compatible = false;
            continue;
        }
        if (output->type != input.type || output->size != input.size) {
            Logger::err(
                "Vertex output and fragment input at location {} are incompatible",
                input.location
            );
            compatible = false;
        }
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
    if (m_handle != VK_NULL_HANDLE && m_device != nullptr) {
        vkDestroyShaderModule(m_device->m_handle, m_handle, Instance::allocator());
        m_handle = VK_NULL_HANDLE;
        m_device = nullptr;
    }
}

ShaderModule::ShaderModule(ShaderModule&& other) noexcept
    : m_device(std::exchange(other.m_device, nullptr))
    , m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    , m_stage(other.m_stage)
    , m_spirv(std::move(other.m_spirv))
    , m_reflected(std::move(other.m_reflected)) {}

auto ShaderModule::operator=(ShaderModule&& other) noexcept -> ShaderModule& {
    if (this != &other) {
        if (m_handle != VK_NULL_HANDLE && m_device != nullptr) {
            vkDestroyShaderModule(m_device->m_handle, m_handle, Instance::allocator());
        }
        m_device = std::exchange(other.m_device, nullptr);
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);

        m_stage = other.m_stage;
        m_spirv = std::move(other.m_spirv);
        m_reflected = std::move(other.m_reflected);
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
    , m_reflected(ReflectedModule::reflect(spirv, m_stage)) {

    const VkShaderModuleCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = spirv.size() * 4, // NOTE: It has to be multiplied by 4!
        .pCode = spirv.data(),
    };

    VkResult result =
        vkCreateShaderModule(m_device->m_handle, &info, Instance::allocator(), &m_handle);
    if (result != VK_SUCCESS) {
        Logger::err("Failed to create Vulkan shader module: {}", to_string(result));
        m_handle = VK_NULL_HANDLE;
    }
}

// Per-object uniform buffers are updated with a dynamic offset. Other sets
// use a regular uniform-buffer descriptor. Set numbers above the engine's
// frequency enum remain valid regular descriptor sets.
static auto get_uniform_buffer_type(u32 set) -> VkDescriptorType {
    if (set == static_cast<u32>(FREQUENCY::PER_OBJECT)) {
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    }
    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

static auto get_sampled_image_type(u32 /*set*/) -> VkDescriptorType {
    return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
}

/*--------------------
    Pipeline Layout
----------------------*/

static auto layout_create_info(
    const std::span<const VkDescriptorSetLayout>& layouts,
    const std::span<const VkPushConstantRange>&   push_constants
) -> VkPipelineLayoutCreateInfo {

    const VkPipelineLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<u32>(layouts.size()),
        .pSetLayouts = layouts.data(),
        .pushConstantRangeCount = static_cast<u32>(push_constants.size()),
        .pPushConstantRanges = push_constants.data(),
    };
    return info;
}

Pipeline::PipelineLayout::~PipelineLayout() {
    if (m_handle != VK_NULL_HANDLE && m_device != nullptr) {
        vkDestroyPipelineLayout(m_device->m_handle, m_handle, Instance::allocator());
        m_handle = VK_NULL_HANDLE;
        m_device = nullptr;
    }
}

Pipeline::PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept
    : m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    , m_device(std::exchange(other.m_device, nullptr)) {}

auto Pipeline::PipelineLayout::operator=(PipelineLayout&& other) noexcept
    -> PipelineLayout& {
    if (this != &other) {
        if (m_handle != VK_NULL_HANDLE && m_device != nullptr) {
            vkDestroyPipelineLayout(m_device->m_handle, m_handle, Instance::allocator());
        }
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
        m_device = std::exchange(other.m_device, nullptr);
    }
    return *this;
}

Pipeline::PipelineLayout::PipelineLayout(
    const LogicalDevice&                               device,
    const std::span<const DescriptorSetLayout>&        set_layouts,
    const std::span<const Pipeline::PushConstantRange>& push_constant_ranges
)
    : m_device(&device) {

    std::vector<VkPushConstantRange> push_constants;
    push_constants.resize(push_constant_ranges.size());
    for (u32 i = 0; i < push_constant_ranges.size(); i++) {
        const Pipeline::PushConstantRange& range = push_constant_ranges[i];
        VkPushConstantRange&               vk_range = push_constants[i];

        vk_range.stageFlags = range.shader_stage;
        vk_range.offset = range.offset;
        vk_range.size = range.size;
    }

    std::vector<VkDescriptorSetLayout> layouts(set_layouts.size(), VK_NULL_HANDLE);
    for (u32 i = 0; i < set_layouts.size(); i++) {
        layouts[i] = set_layouts[i].m_handle;
    }

    const VkPipelineLayoutCreateInfo info = layout_create_info(layouts, push_constants);
    VkResult                         result =
        vkCreatePipelineLayout(device.m_handle, &info, Instance::allocator(), &m_handle);
    if (result != VK_SUCCESS) {
        Logger::err("Failed to create Vulkan pipeline layout: {}", to_string(result));
        m_handle = VK_NULL_HANDLE;
        return;
    }

#if 0
    // Logging the pipeline layout
    {
        std::string offset = "  ";
        Logger::trace("Created Pipeline Layout {} at {}", fmt::ptr(this), fmt::ptr(m_handle));
        Logger::trace("\tSet Layouts count: {}", set_layouts.size());
        for (u32 i = 0; i < set_layouts.size(); i++) {
            std::string layout_str;
            switch (i) {
                case 0: layout_str = "Global"; break;
                case 1: layout_str = "Per-Pass"; break;
                case 2: layout_str = "Per-Material"; break;
                case 3: layout_str = "Per-Object"; break;
                default: layout_str = "Unknown"; break;
            }
            Logger::trace("\t-Set layout: {} {}", i, layout_str);
            const DescriptorSetLayout& set_layout = set_layouts[i];
            for (u32 j = 0; j < set_layout.m_bindings.size(); j++) {
                const VkDescriptorSetLayoutBinding& binding = set_layout.m_bindings[j];
                Logger::trace("\t\tBinding: {}", binding.binding);
                Logger::trace("\t\t-Type: {}", to_string(binding.descriptorType));
                Logger::trace("\t\t-Count: {}", binding.descriptorCount);
                Logger::trace("\t\t-Stage: {}", to_string_from_shader_stage_flags(binding.stageFlags));
            }
        }
        Logger::trace("Push constant ranges count: {}", push_constant_ranges.size());
        for (u32 i = 0; i < push_constant_ranges.size(); i++) {
            const VkPushConstantRange& push_constant_range = push_constant_ranges[i];
            Logger::trace("\tPush constant range: {}", i);
            Logger::trace("\t\tStage: {}", to_string_from_shader_stage_flags(push_constant_range.stageFlags));
            Logger::trace("\t\tOffset: {}", push_constant_range.offset);
            Logger::trace("\t\tSize: {}", push_constant_range.size);
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
    const VkPipelineShaderStageCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = stage,
        .module = shader_module,
        .pName = "main",
        .pSpecializationInfo = nullptr,
    };
    return info;
}

// Viewport and scissor are dynamic state, only their count is part of the pipeline.
static auto dynamic_viewport_state_create_info() -> VkPipelineViewportStateCreateInfo {
    const VkPipelineViewportStateCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };
    return info;
}

static auto get_binding_description(const VertexFormat& vertex_format)
    -> VkVertexInputBindingDescription {
    u32 stride = 0;
    for (const VertexAttribute& attribute : vertex_format.m_attributes) {
        stride += get_size(attribute.type);
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
    std::vector<VkVertexInputAttributeDescription> attribs;
    attribs.resize(vertex_format.m_attributes.size());

    size_t offset = 0;
    for (u32 i = 0; i < vertex_format.m_attributes.size(); i++) {
        attribs[i].offset = offset;
        auto type_size = get_size(vertex_format.m_attributes[i].type);
        offset += type_size;
        attribs[i].binding = 0;
        attribs[i].location = vertex_format.m_attributes[i].location;
        attribs[i].format = to_vk_vertex_format(vertex_format.m_attributes[i].type);
    }

    return attribs;
}

static auto vertex_input_state_create_info(
    const VkVertexInputBindingDescription&             bindings,
    const std::span<VkVertexInputAttributeDescription> attributes
) -> VkPipelineVertexInputStateCreateInfo {

    const VkPipelineVertexInputStateCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindings,
        .vertexAttributeDescriptionCount = static_cast<u32>(attributes.size()),
        .pVertexAttributeDescriptions = attributes.data(),
    };
    return info;
}

static auto input_assembly_state_create_info(VkPrimitiveTopology topology)
    -> VkPipelineInputAssemblyStateCreateInfo {
    const VkPipelineInputAssemblyStateCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = topology,
        .primitiveRestartEnable = VK_FALSE,
    };
    return info;
}

static auto rasterization_state_create_info() -> VkPipelineRasterizationStateCreateInfo {
    auto polygon_mode = VK_POLYGON_MODE_FILL;
    // NOTE: In debugging one might want to use VK_CULL_MODE_NONE, as opposed to
    // VK_CULL_MODE_BACK_BIT
    VkCullModeFlags cull_mode = VK_CULL_MODE_NONE;
    auto            front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    const VkPipelineRasterizationStateCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = polygon_mode,
        .cullMode = cull_mode,
        .frontFace = front_face,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = {},
        .depthBiasClamp = {},
        .depthBiasSlopeFactor = {},
        .lineWidth = 1.0F,
    };
    return info;
}

static auto multisample_state_create_info() -> VkPipelineMultisampleStateCreateInfo {
    const VkPipelineMultisampleStateCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0F,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };
    return info;
}

static auto color_blend_attachment_state() -> VkPipelineColorBlendAttachmentState {

    VkColorComponentFlags color_write_mask = {};
    color_write_mask |= VK_COLOR_COMPONENT_R_BIT;
    color_write_mask |= VK_COLOR_COMPONENT_G_BIT;
    color_write_mask |= VK_COLOR_COMPONENT_B_BIT;
    color_write_mask |= VK_COLOR_COMPONENT_A_BIT;
    const VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = color_write_mask,
    };
    // const VkPipelineColorBlendAttachmentState color_blend_attachment = {
    //     .blendEnable = VK_FALSE,
    //     .srcColorBlendFactor = {},
    //     .dstColorBlendFactor = {},
    //     .colorBlendOp = {},
    //     .srcAlphaBlendFactor = {},
    //     .dstAlphaBlendFactor = {},
    //     .alphaBlendOp = {},
    //     .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    //                       VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    // };
    return color_blend_attachment;
}

static auto color_blend_state_create_info(
    const VkPipelineColorBlendAttachmentState& color_blend_attachment
) -> VkPipelineColorBlendStateCreateInfo {
    const VkPipelineColorBlendStateCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants = {0.0F, 0.0F, 0.0F, 0.0F},
    };
    return info;
}

Pipeline::Pipeline(Pipeline&& other) noexcept
    : m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    , m_layout(std::exchange(other.m_layout, {}))
    , m_device(std::exchange(other.m_device, nullptr))
    , m_render_pass(std::exchange(other.m_render_pass, nullptr))
    , m_set_layouts(std::exchange(other.m_set_layouts, {}))
    , m_code(std::exchange(other.m_code, {}))
    , m_is_compiled(std::exchange(other.m_is_compiled, false))
    , m_push_constant_ranges(std::exchange(other.m_push_constant_ranges, {}))
    , m_reflected_code(std::exchange(other.m_reflected_code, {}))
    , m_reflected_interface(std::exchange(other.m_reflected_interface, {})) {}

auto Pipeline::operator=(Pipeline&& other) noexcept -> Pipeline& {
    if (this != &other) {
        if (m_handle != VK_NULL_HANDLE && m_device != nullptr) {
            vkDestroyPipeline(m_device->m_handle, m_handle, Instance::allocator());
        }
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
        m_layout = std::exchange(other.m_layout, {});
        m_device = std::exchange(other.m_device, nullptr);
        m_render_pass = std::exchange(other.m_render_pass, nullptr);
        m_set_layouts = std::exchange(other.m_set_layouts, {});
        m_code = std::exchange(other.m_code, {});
        m_is_compiled = std::exchange(other.m_is_compiled, false);
        m_push_constant_ranges = std::exchange(other.m_push_constant_ranges, {});
        m_reflected_code = std::exchange(other.m_reflected_code, {});
        m_reflected_interface = std::exchange(other.m_reflected_interface, {});
    }
    return *this;
}

Pipeline::~Pipeline() {
    if (m_handle != VK_NULL_HANDLE && m_device != nullptr) {
        vkDestroyPipeline(m_device->m_handle, m_handle, Instance::allocator());
        m_handle = VK_NULL_HANDLE;
        m_device = nullptr;
    }
}

static auto get_reflected_modules(const std::vector<ShaderModule>& modules)
    -> std::vector<ReflectedModule> {
    std::vector<ReflectedModule> reflected_modules;
    reflected_modules.resize(modules.size());
    for (u32 i = 0; i < modules.size(); i++) {
        reflected_modules[i] = modules[i].m_reflected;
    }
    return reflected_modules;
}

struct NormalizedDescriptorBinding {
    u32                                               set = 0;
    u32                                               binding = 0;
    VkDescriptorType                                  descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    u32                                               descriptor_count = 0;
    VkShaderStageFlags                                stage_flags = 0;
    const ReflectedModule::UniformBuffer*             uniform_buffer = nullptr;
};

static auto uniform_layouts_match(
    const ReflectedModule::UniformBuffer& lhs,
    const ReflectedModule::UniformBuffer& rhs
) -> bool {
    if (lhs.size != rhs.size || lhs.descriptor_count != rhs.descriptor_count ||
        lhs.members.size() != rhs.members.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.members.size(); ++i) {
        const auto& lhs_member = lhs.members[i];
        const auto& rhs_member = rhs.members[i];
        if (lhs_member.offset != rhs_member.offset || lhs_member.size != rhs_member.size ||
            lhs_member.type != rhs_member.type) {
            return false;
        }
    }
    return true;
}

static auto extract_descriptor_set_layouts(
    const std::span<const ReflectedModule> modules,
    const LogicalDevice&                   device,
    std::vector<DescriptorSetLayout>&      set_layouts
) -> bool {
    using BindingKey = std::pair<u32, u32>;

    // The map is the normalized reflection representation used to create the
    // Vulkan layouts. A key can occur in multiple shader stages, but only one
    // Vulkan binding is emitted for it.
    std::map<BindingKey, NormalizedDescriptorBinding> normalized;
    u32 max_set = 0;
    bool has_bindings = false;

    auto add_binding = [&](u32 set,
                           u32 binding,
                           VkDescriptorType descriptor_type,
                           u32 descriptor_count,
                           VkShaderStageFlags stage_flags,
                           const ReflectedModule::UniformBuffer* uniform_buffer) -> bool {
        if (descriptor_count == 0) {
            Logger::err(
                "Descriptor set {} binding {} has an unresolved or runtime array size",
                set,
                binding
            );
            return false;
        }
        // DescriptorSet currently exposes scalar bind operations. Rejecting
        // arrays here prevents a valid Vulkan layout from being paired with
        // an incapable material implementation.
        if (descriptor_count != 1) {
            Logger::err(
                "Descriptor set {} binding {} uses {} descriptors; descriptor arrays are "
                "not supported by Vulkan_Material",
                set,
                binding,
                descriptor_count
            );
            return false;
        }

        const BindingKey key = {set, binding};
        const auto [it, inserted] = normalized.emplace(
            key,
            NormalizedDescriptorBinding{
                .set = set,
                .binding = binding,
                .descriptor_type = descriptor_type,
                .descriptor_count = descriptor_count,
                .stage_flags = stage_flags,
                .uniform_buffer = uniform_buffer,
            }
        );
        if (inserted) {
            max_set = std::max(max_set, set);
            has_bindings = true;
            return true;
        }

        auto& existing = it->second;
        if (existing.descriptor_type != descriptor_type) {
            Logger::err(
                "Descriptor set {} binding {} has incompatible descriptor types ({}, {})",
                set,
                binding,
                to_string(existing.descriptor_type),
                to_string(descriptor_type)
            );
            return false;
        }
        if (existing.descriptor_count != descriptor_count) {
            Logger::err(
                "Descriptor set {} binding {} has incompatible array counts ({}, {})",
                set,
                binding,
                existing.descriptor_count,
                descriptor_count
            );
            return false;
        }
        if (existing.uniform_buffer != nullptr && uniform_buffer != nullptr &&
            !uniform_layouts_match(*existing.uniform_buffer, *uniform_buffer)) {
            Logger::err(
                "Uniform buffer set {} binding {} has incompatible layouts between shader "
                "stages",
                set,
                binding
            );
            return false;
        }
        existing.stage_flags |= stage_flags;
        return true;
    };

    for (const auto& module : modules) {
        const VkShaderStageFlags stage = to_ShaderStageFlagsBits(module.m_stage);
        for (const auto& buffer : module.m_uniform_buffers) {
            if (!add_binding(
                    buffer.set,
                    buffer.binding,
                    get_uniform_buffer_type(buffer.set),
                    buffer.descriptor_count,
                    stage,
                    &buffer
                )) {
                return false;
            }
        }
        for (const auto& image : module.m_sampled_images) {
            if (!add_binding(
                    image.set,
                    image.binding,
                    get_sampled_image_type(image.set),
                    image.descriptor_count,
                    stage,
                    nullptr
                )) {
                return false;
            }
        }
    }

    set_layouts.clear();
    if (!has_bindings) { return true; }

    // Vulkan requires layouts for every set below the highest referenced set,
    // including empty layouts for gaps. The number is determined by reflection
    // rather than by FREQUENCY::MAX.
    std::vector<std::vector<DescriptorSetLayout::Binding>> bindings(max_set + 1);
    for (const auto& [key, descriptor] : normalized) {
        (void)key;
        bindings[descriptor.set].push_back(
            DescriptorSetLayout::Binding{
                .binding = descriptor.binding,
                .type = descriptor.descriptor_type,
                .count = descriptor.descriptor_count,
                .stage_flags = descriptor.stage_flags,
            }
        );
    }

    set_layouts.resize(max_set + 1);
    for (u32 set = 0; set <= max_set; ++set) {
        set_layouts[set] = device.create_descriptor_set_layout(bindings[set]);
    }
    return true;
}

Pipeline::Pipeline(
    const LogicalDevice& device,
    const RenderPass&    render_pass,
    const ShadingCode&   code
)
    : m_device(&device)
    , m_render_pass(&render_pass) {

    m_code = code;
    std::vector<ShaderModule> modules;
    modules.resize(code.m_modules.size());
    for (u32 i = 0; i < modules.size(); i++) {
        const auto& module_ = code.m_modules[i];
        const auto& code_ = module_.m_code;
        const auto& stage_ = module_.m_stage;
        modules[i] = ShaderModule(device, code_, stage_);
        if (modules[i].m_handle == VK_NULL_HANDLE) {
            Logger::err("Failed to create a Vulkan shader module for the graphics pipeline");
            return;
        }
    }

    auto reflected_modules = get_reflected_modules(modules);
    m_reflected_code.m_modules = reflected_modules;
    m_reflected_interface = ReflectedModule::into_interface(reflected_modules);
    VertexFormat vertex_format = m_reflected_interface.get_vertex_format();

    bool has_vertex = false;
    bool has_fragment = false;
    for (const auto& module : reflected_modules) {
        if (module.m_stage == SHADER_STAGE::VERTEX) {
            if (has_vertex) {
                Logger::err("A graphics pipeline contains multiple vertex shader modules");
                return;
            }
            has_vertex = true;
        } else if (module.m_stage == SHADER_STAGE::FRAGMENT) {
            if (has_fragment) {
                Logger::err("A graphics pipeline contains multiple fragment shader modules");
                return;
            }
            has_fragment = true;
        } else {
            Logger::err(
                "Shader stage {} is not supported by the Vulkan graphics pipeline",
                to_string(module.m_stage)
            );
            return;
        }
    }
    if (!has_vertex || !has_fragment) {
        Logger::err("A Vulkan graphics pipeline requires vertex and fragment shader modules");
        return;
    }

    std::vector<VkPipelineShaderStageCreateInfo> stages;
    stages.resize(m_code.m_modules.size());
    for (u32 i = 0; i < stages.size(); i++) {
        stages[i] = shader_stage_create_info(
            to_ShaderStageFlagsBits(modules[i].m_stage), modules[i].m_handle
        );
    }

    // The renderer sets viewport and scissor while recording, see
    // `vulkan::to_viewport_state`.
    const std::array<VkDynamicState, 2> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    const VkPipelineDynamicStateCreateInfo dynamic_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<u32>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data(),
    };

    const VkPipelineInputAssemblyStateCreateInfo input_assembly =
        input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    const VkPipelineRasterizationStateCreateInfo rasterizer =
        rasterization_state_create_info();
    const VkPipelineMultisampleStateCreateInfo multisampling =
        multisample_state_create_info();
    const VkPipelineColorBlendAttachmentState color_blend_attachment =
        color_blend_attachment_state();
    const VkPipelineViewportStateCreateInfo viewport_state =
        dynamic_viewport_state_create_info();
    const VkPipelineColorBlendStateCreateInfo color_blending =
        color_blend_state_create_info(color_blend_attachment);

    const VkVertexInputBindingDescription binding_description =
        get_binding_description(vertex_format);
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions =
        get_attribute_descriptions(vertex_format);
    if (!check_compatibility(reflected_modules, binding_description, attribute_descriptions)) {
        Logger::err("Shader reflection is incompatible with the Vulkan vertex input layout");
        return;
    }

    if (!extract_descriptor_set_layouts(reflected_modules, device, m_set_layouts)) {
        Logger::err("Shader reflection is incompatible with the Vulkan descriptor layout");
        return;
    }

    m_layout = PipelineLayout(device, m_set_layouts, m_push_constant_ranges);
    if (m_layout.m_handle == VK_NULL_HANDLE) {
        Logger::err("Failed to create the Vulkan pipeline layout from shader reflection");
        return;
    }

    const VkPipelineVertexInputStateCreateInfo vertex_input_info =
        vertex_input_state_create_info(binding_description, attribute_descriptions);

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0F,
        .maxDepthBounds = 1.0F,
    };

    const VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = static_cast<u32>(stages.size()),
        .pStages = stages.data(),
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depth_stencil,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
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
    if (result != VK_SUCCESS) {
        Logger::err("Failed to create Vulkan graphics pipeline: {}", to_string(result));
        m_handle = VK_NULL_HANDLE;
        return;
    }
    // Logger part
    {
        Logger::trace(
            "Created graphics pipeline {} at {}", fmt::ptr(this), fmt::ptr(m_handle)
        );
        // Logger::trace("\tShader stages:");
        // for (const auto& stage : shader_stages) {
        //     Logger::trace("\t\t{}", stage.module);
        // }
    }

    m_is_compiled = true;
}

} // namespace vulkan
} // namespace JadeFrame
