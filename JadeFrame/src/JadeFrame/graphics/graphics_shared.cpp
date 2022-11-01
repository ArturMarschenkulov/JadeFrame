#include "pch.h"
#include "graphics_shared.h"
JF_PRAGMA_PUSH
#pragma warning(disable : 4006)
#include "shaderc/shaderc.hpp"
JF_PRAGMA_POP

#include "SPIRV-Cross/spirv_glsl.hpp"
#include "SPIRV-Cross/spirv_hlsl.hpp"
#include "SPIRV-Cross/spirv_msl.hpp"

#include "JadeFrame/utils/assert.h"
namespace JadeFrame {

VertexAttribute::VertexAttribute(const std::string& name, SHADER_TYPE type, bool normalized)
    : name(name)
    , type(type)
    , size(SHADER_TYPE_get_size(type))
    , offset(0)
    , normalized(normalized) {}

VertexFormat::VertexFormat(const std::initializer_list<VertexAttribute>& attributes)
    : m_attributes(attributes) {
    this->calculate_offset_and_stride(m_attributes);
}
auto VertexFormat::calculate_offset_and_stride(std::vector<VertexAttribute>& attributes) -> void {
    size_t offset = 0;
    m_stride = 0;
    for (VertexAttribute& attribute : attributes) {
        attribute.offset = offset;
        offset += attribute.size;
        m_stride += attribute.size;
    }
}



auto string_to_SPIRV(const std::string& code, SHADER_STAGE stage) -> std::vector<u32> {
    shaderc_shader_kind kind = {};
    switch (stage) {
        case SHADER_STAGE::VERTEX: {
            kind = shaderc_vertex_shader;
        } break;
        case SHADER_STAGE::FRAGMENT: {
            kind = shaderc_fragment_shader;
        } break;
        default: assert(false);
    }

    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
    // options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
    options.SetWarningsAsErrors();
    options.SetGenerateDebugInfo();
    const bool optimize = false;
    if constexpr (optimize == true) { options.SetOptimizationLevel(shaderc_optimization_level_size); }
    shaderc::Compiler             compiler;
    shaderc::SpvCompilationResult comp_result = compiler.CompileGlslToSpv(code, kind, "", options);
    shaderc_compilation_status    comp_status = comp_result.GetCompilationStatus();
    if (comp_status != shaderc_compilation_status_success) {
        assert(false);
        return std::vector<u32>();
    }

    std::vector<u32> result = {comp_result.cbegin(), comp_result.cend()};
    return result;
}


class RenderCommandQueue {
public:
    typedef void (*RenderCommandFn)(void*);
    RenderCommandQueue() {
        const auto buffer_size = 10 * 1024 * 1024;
        m_command_buffer = new u8[buffer_size];
        m_command_buffer_ptr = m_command_buffer;
        std::memset(m_command_buffer, 0, buffer_size);
    }
    ~RenderCommandQueue() { delete[] m_command_buffer; }
    auto allocate(RenderCommandFn func, u32 size) {
        // TODO: alignment
        *(RenderCommandFn*)m_command_buffer_ptr = func;
        m_command_buffer_ptr += sizeof(RenderCommandFn);

        *(u32*)m_command_buffer_ptr = size;
        m_command_buffer_ptr += sizeof(u32);

        void* memory = m_command_buffer_ptr;
        m_command_buffer_ptr += size;

        m_command_count++;
        return memory;
    }
    auto execute() -> void {

        u8* buffer = m_command_buffer;

        for (uint32_t i = 0; i < m_command_count; i++) {
            RenderCommandFn function = *(RenderCommandFn*)buffer;
            buffer += sizeof(RenderCommandFn);

            u32 size = *(u32*)buffer;
            buffer += sizeof(u32);
            function(buffer);
            buffer += size;
        }

        m_command_buffer_ptr = m_command_buffer;
        m_command_count = 0;
    }

private:
    u8* m_command_buffer;
    u8* m_command_buffer_ptr;
    u32 m_command_count = 0;
};



// --------------------------------------------

static auto debug_print_resources(const spirv_cross::ShaderResources& resources) -> void {
    Logger::info("printing shader resources");
    for (const spirv_cross::Resource& resource : resources.uniform_buffers) {
        const std::string& name = resource.name;
        Logger::info("\tuniform_buffers {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.storage_buffers) {
        const std::string& name = resource.name;
        Logger::info("\tstorage_buffers {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.stage_inputs) {
        const std::string& name = resource.name;
        Logger::info("\tstage_inputs {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.stage_outputs) {
        const std::string& name = resource.name;
        Logger::info("\tstage_outputs {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.subpass_inputs) {
        const std::string& name = resource.name;
        Logger::info("\tsubpass_inputs {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.storage_images) {
        const std::string& name = resource.name;
        Logger::info("\tstorage_images {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.sampled_images) {
        const std::string& name = resource.name;
        Logger::info("\tsampled_images {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.atomic_counters) {
        const std::string& name = resource.name;
        Logger::info("\tatomic_counters {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.acceleration_structures) {
        const std::string& name = resource.name;
        Logger::info("\tacceleration_structures {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.push_constant_buffers) {
        const std::string& name = resource.name;
        Logger::info("\tpush_constant_buffers {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.separate_images) {
        const std::string& name = resource.name;
        Logger::info("\tseparate_images {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.separate_samplers) {
        const std::string& name = resource.name;
        Logger::info("\tseparate_samplers {}", name);
    }

    for (const spirv_cross::BuiltInResource& resource : resources.builtin_inputs) {
        const std::string& name = resource.resource.name;
        Logger::info("\tbuiltin_inputs {}", name);
    }
    for (const spirv_cross::BuiltInResource& resource : resources.builtin_outputs) {
        const std::string& name = resource.resource.name;
        Logger::info("\tbuiltin_outputs {}", name);
    }
}

static auto to_SHADER_TYPE(const spirv_cross::SPIRType& type, u32 rows, u32 columns) -> SHADER_TYPE {
    SHADER_TYPE result = SHADER_TYPE::NONE;
    if (columns == 1) {
        switch (type.basetype) {
            case spirv_cross::SPIRType::Float: {

                SHADER_TYPE arr[] = {
                    SHADER_TYPE::FLOAT, SHADER_TYPE::FLOAT_2, SHADER_TYPE::FLOAT_3, SHADER_TYPE::FLOAT_4};
                if (rows < 5) {
                    result = arr[rows - 1];
                } else {
                    JF_ASSERT(false, "this should not be reached!");
                }

            } break;
            default: JF_ASSERT(false, "this should not be reached!");
        }
    } else {
        JF_ASSERT(false, "not implemented yet!");
    }
    return result;
}

auto reflect(const ShadingCode& code) -> ReflectedCode {
    ReflectedCode result = {};

    result.m_modules.resize(code.m_modules.size());
    for (u32 i = 0; i < code.m_modules.size(); i++) {
        auto& current_module = code.m_modules[i];
        auto& current_module_code = std::get<std::vector<u32>>(current_module.m_code);
        auto& current_result_module = result.m_modules[i];

        spirv_cross::Compiler        compiler(current_module_code);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        debug_print_resources(resources);

        result.m_modules[i].m_inputs.resize(resources.stage_inputs.size());
        for (u32 j = 0; j < resources.stage_inputs.size(); j++) {
            const spirv_cross::Resource& resource = resources.stage_inputs[j];

            const std::string& name = resource.name;

            const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
            const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
            i32                          member_count = static_cast<u32>(buffer_type.member_types.size());
            u32                          location = compiler.get_decoration(resource.id, spv::DecorationLocation);
            u32                          size = (buffer_type.width / 8) * buffer_type.vecsize * buffer_type.columns;


            std::vector<ReflectedCode::Input>& inputs = current_result_module.m_inputs;
            inputs[j].name = name;
            inputs[j].location = location;
            inputs[j].size = size;
            inputs[j].type = to_SHADER_TYPE(buffer_type, buffer_type.vecsize, buffer_type.columns);
        }

        result.m_modules[i].m_uniform_buffers.resize(resources.uniform_buffers.size());
        for (u32 j = 0; j < resources.uniform_buffers.size(); j++) {
            const spirv_cross::Resource& resource = resources.uniform_buffers[j];

            const std::string&           name = resource.name;
            const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.base_type_id);
            i32                          member_count = static_cast<u32>(buffer_type.member_types.size());
            u32                          binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            u32                          set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            u32 size = static_cast<u32>(compiler.get_declared_struct_size(buffer_type));

            std::vector<ReflectedCode::UniformBuffer>& uniform_buffers = current_result_module.m_uniform_buffers;
            uniform_buffers[j].binding = binding;
            uniform_buffers[j].set = set;
            uniform_buffers[j].name = name;
            uniform_buffers[j].size = size;
        }


        result.m_modules[i].m_sampled_images.resize(resources.sampled_images.size());
        for (u32 j = 0; j < resources.sampled_images.size(); j++) {
            const spirv_cross::Resource& resource = resources.sampled_images[j];

            const std::string&           name = resource.name;
            const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
            const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
            i32                          member_count = static_cast<u32>(buffer_type.member_types.size());
            u32                          binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            u32                          set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

            u32 dimension = base_type.image.dim;
            u32 array_size = buffer_type.array[0];

            if (array_size == 0) {
                array_size = 1;
            } else {
                assert(false);
            }
            //__debugbreak();
        }


        // result.m_modules[i].m_push_constant_ranges.resize(resources.push_constant_buffers.size());
        // for (u32 j = 0; j < resources.push_constant_buffers.size(); j++) {
        //     const spirv_cross::Resource& resource = resources.push_constant_buffers[j];

        //     const std::string&           buffer_name = resource.name;
        //     const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.base_type_id);
        //     u32                          buffer_size = (u32)compiler.get_declared_struct_size(buffer_type);
        //     u32                          member_count = uint32_t(buffer_type.member_types.size());
        //     u32                          buffer_offset = 0;

        //     std::vector<VkPushConstantRange>& push_constant_ranges = current_result_module.m_push_constant_ranges;
        //     push_constant_ranges[j].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        //     push_constant_ranges[j].size = buffer_size - buffer_offset;
        //     push_constant_ranges[j].offset = buffer_offset;
        // }
    }
    return result;
}
} // namespace JadeFrame