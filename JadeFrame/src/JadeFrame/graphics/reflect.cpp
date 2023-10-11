#include "reflect.h"

#include "JadeFrame/utils/assert.h"

JF_PRAGMA_PUSH
#pragma warning(disable : 4006)
#include "shaderc/shaderc.hpp"
JF_PRAGMA_POP

#include "SPIRV-Cross/spirv_glsl.hpp"
#include "SPIRV-Cross/spirv_hlsl.hpp"
#include "SPIRV-Cross/spirv_msl.hpp"

namespace JadeFrame {

auto convert_SPIRV_to_opengl(const ShadingCode& code) -> ShadingCode {

    spirv_cross::CompilerGLSL::Options options;
    options.version = 450;
    options.es = false;
    options.vulkan_semantics = true;
    spirv_cross::CompilerGLSL compiler = spirv_cross::CompilerGLSL(code.m_modules[0].m_code);

    compiler.set_common_options(options);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    std::array<std::vector<spirv_cross::Resource*>, 4> dd;
    for (u32 j = 0; j < resources.uniform_buffers.size(); j++) {
        spirv_cross::Resource& resource = resources.uniform_buffers[j];

        u32 set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        JF_ASSERT(set <= 3, "As of right now, only 4 descriptor sets are supported. (0, 1, 2, 3)");

        dd[set].push_back(&resource);
    }

    u32 binding = 0;
    for (u32 i = 0; i < dd.size(); i++) {
        for (u32 j = 0; j < dd[i].size(); j++) {
            // compiler.unset_decoration(dd[i][j]->id, spv::DecorationDescriptorSet);
            compiler.set_decoration(dd[i][j]->id, spv::DecorationDescriptorSet, 0);
            compiler.set_decoration(dd[i][j]->id, spv::DecorationBinding, binding);
            binding++;
        }
    }
    auto source = compiler.compile();

    auto new_code = code;
    new_code.m_modules[0].m_code = string_to_SPIRV(source, SHADER_STAGE::VERTEX, GRAPHICS_API::OPENGL);
    return new_code;
}


auto string_to_SPIRV(const std::string& code, SHADER_STAGE stage, GRAPHICS_API api) -> std::vector<u32> {
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
    switch (api) {
        case GRAPHICS_API::VULKAN: {
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        } break;
        case GRAPHICS_API::OPENGL: {
            options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
        } break;
        default: JF_UNIMPLEMENTED("");
    }
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
    if (rows > 4) { JF_PANIC("rows must be less than 4"); }
    if (columns > 4) { JF_PANIC("columns must be less than 4"); }


    if (columns == 1) {
        switch (type.basetype) {
            case spirv_cross::SPIRType::Float: {
                SHADER_TYPE arr[] = {
                    SHADER_TYPE::F32, SHADER_TYPE::V_2_F32, SHADER_TYPE::V_3_F32, SHADER_TYPE::V_4_F32};
                result = arr[rows - 1];
            } break;
            default: JF_ASSERT(false, "this should not be reached!");
        }
    } else {
        switch (type.basetype) {
            case spirv_cross::SPIRType::Float: {
                if (columns == rows) {
                    JF_UNIMPLEMENTED("matrix types with different row and column count are not supported yet!");
                }
                SHADER_TYPE arr[] = {SHADER_TYPE::M_2_2_F32, SHADER_TYPE::M_3_3_F32, SHADER_TYPE::M_4_4_F32};

                result = arr[rows - 2];
            } break;
            default: JF_ASSERT(false, "this should not be reached!");
        }
    }
    return result;
}

auto temp_cmp_0(const ReflectedCode::Input& i0, const ReflectedCode::Input& i1) -> bool {
    return i0.location < i1.location;
}
auto temp_cmp_1(const ReflectedModule::Input& i0, const ReflectedModule::Input& i1) -> bool {
    return i0.location < i1.location;
}
auto temp_cmp(ReflectedCode::Output i0, ReflectedCode::Output i1) -> bool { return i0.location < i1.location; }

auto reflect(const ShadingCode::Module::SPIRV& code) -> ReflectedModule {
    ReflectedModule result = {};
    // result.m_stage = current_module.m_stage;

    spirv_cross::Compiler        compiler(code);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    debug_print_resources(resources);

    result.m_inputs.resize(resources.stage_inputs.size());
    for (u32 j = 0; j < resources.stage_inputs.size(); j++) {
        const spirv_cross::Resource& resource = resources.stage_inputs[j];


        const std::string& name = resource.name;

        const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
        const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
        i32                          member_count = static_cast<u32>(buffer_type.member_types.size());
        u32                          location = compiler.get_decoration(resource.id, spv::DecorationLocation);
        u32                          size = (buffer_type.width / 8) * buffer_type.vecsize * buffer_type.columns;

        std::vector<ReflectedModule::Input>& inputs = result.m_inputs;
        inputs[j].name = name;
        inputs[j].location = location;
        inputs[j].size = size;
        inputs[j].type = to_SHADER_TYPE(buffer_type, buffer_type.vecsize, buffer_type.columns);
    }



    result.m_outputs.resize(resources.stage_inputs.size());
    for (u32 j = 0; j < resources.stage_outputs.size(); j++) {
        const spirv_cross::Resource& resource = resources.stage_outputs[j];

        const std::string& name = resource.name;



        const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
        const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
        i32                          member_count = static_cast<u32>(buffer_type.member_types.size());
        u32                          location = compiler.get_decoration(resource.id, spv::DecorationLocation);
        u32                          size = (buffer_type.width / 8) * buffer_type.vecsize * buffer_type.columns;

        std::vector<ReflectedModule::Output>& outputs = result.m_outputs;
        outputs[j].name = name;
        outputs[j].location = location;
        outputs[j].size = size;
        outputs[j].type = to_SHADER_TYPE(buffer_type, buffer_type.vecsize, buffer_type.columns);
    }

    result.m_uniform_buffers.resize(resources.uniform_buffers.size());
    for (u32 j = 0; j < resources.uniform_buffers.size(); j++) {
        const spirv_cross::Resource& resource = resources.uniform_buffers[j];

        const std::string&           name = resource.name;
        const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
        const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);

        Logger::info(
            "the uniform buffer {} has {} members. base id {}, id {}", name, buffer_type.member_types.size(),
            resource.base_type_id, resource.type_id);

        u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        u32 set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        u32 size = static_cast<u32>(compiler.get_declared_struct_size(buffer_type));

        std::vector<ReflectedModule::UniformBuffer>& uniform_buffers = result.m_uniform_buffers;
        uniform_buffers[j].binding = binding;
        uniform_buffers[j].set = set;
        uniform_buffers[j].name = name;
        uniform_buffers[j].size = size;

        JF_ASSERT(base_type.member_types.size() > 0, "this uniform buffer doesn't have any members");
        for (u32 jj = 0; jj < base_type.member_types.size(); jj++) {
            const spirv_cross::SPIRType& member_type = compiler.get_type(base_type.member_types[jj]);
            const std::string&           member_name = compiler.get_member_name(resource.base_type_id, jj);
            u32 member_size = static_cast<u32>(compiler.get_declared_struct_member_size(buffer_type, jj));
            u32 member_offset = static_cast<u32>(compiler.type_struct_member_offset(buffer_type, jj));


            ReflectedModule::UniformBuffer::Member member = {};
            member.name = member_name;
            member.size = member_size;
            member.offset = member_offset;
            member.type = to_SHADER_TYPE(member_type, member_type.vecsize, member_type.columns);
            Logger::info(
                "\tthe member {}.{} has, type {}, size {} and offset {}", name, member_name, to_string(member.type),
                member_size, member_offset);

            uniform_buffers[j].members.push_back(member);
        }
        compiler.get_member_name(resource.base_type_id, 0);
        compiler.get_declared_struct_size(compiler.get_type(resource.base_type_id));
    }


    result.m_sampled_images.resize(resources.sampled_images.size());
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
        std::vector<ReflectedModule::SampledImage>& sampled_images = result.m_sampled_images;
        sampled_images[j].binding = binding;
        sampled_images[j].set = set;
        sampled_images[j].name = name;
    }
    std::sort(result.m_inputs.begin(), result.m_inputs.end(), temp_cmp_1);
    // std::sort(result.m_outputs.begin(), result.m_outputs.end(), temp_cmp);



    // result.m_push_constant_ranges.resize(resources.push_constant_buffers.size());
    // for (u32 j = 0; j < resources.push_constant_buffers.size(); j++) {
    //     const spirv_cross::Resource& resource = resources.push_constant_buffers[j];

    //     const std::string&           buffer_name = resource.name;
    //     const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.base_type_id);
    //     u32                          buffer_size = (u32)compiler.get_declared_struct_size(buffer_type);
    //     u32                          member_count = uint32_t(buffer_type.member_types.size());
    //     u32                          buffer_offset = 0;

    //     std::vector<VkPushConstantRange>& push_constant_ranges = result.m_push_constant_ranges;
    //     push_constant_ranges[j].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    //     push_constant_ranges[j].size = buffer_size - buffer_offset;
    //     push_constant_ranges[j].offset = buffer_offset;
    // }
    return result;
}
auto reflect(const ShadingCode& code) -> ReflectedCode {
    ReflectedCode result = {};

    result.m_modules.resize(code.m_modules.size());
    for (u32 i = 0; i < code.m_modules.size(); i++) {
        auto& current_module = code.m_modules[i];
        auto& current_module_code = current_module.m_code;
        auto& current_result_module = result.m_modules[i];
        result.m_modules[i].m_stage = current_module.m_stage;

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



        result.m_modules[i].m_outputs.resize(resources.stage_inputs.size());
        for (u32 j = 0; j < resources.stage_outputs.size(); j++) {
            const spirv_cross::Resource& resource = resources.stage_outputs[j];

            const std::string& name = resource.name;



            const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
            const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
            i32                          member_count = static_cast<u32>(buffer_type.member_types.size());
            u32                          location = compiler.get_decoration(resource.id, spv::DecorationLocation);
            u32                          size = (buffer_type.width / 8) * buffer_type.vecsize * buffer_type.columns;

            std::vector<ReflectedCode::Output>& outputs = current_result_module.m_outputs;
            outputs[j].name = name;
            outputs[j].location = location;
            outputs[j].size = size;
            outputs[j].type = to_SHADER_TYPE(buffer_type, buffer_type.vecsize, buffer_type.columns);
        }

        result.m_modules[i].m_uniform_buffers.resize(resources.uniform_buffers.size());
        for (u32 j = 0; j < resources.uniform_buffers.size(); j++) {
            const spirv_cross::Resource& resource = resources.uniform_buffers[j];

            const std::string&           name = resource.name;
            const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
            const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);

            Logger::info(
                "the uniform buffer {} has {} members. base id {}, id {}", name, buffer_type.member_types.size(),
                resource.base_type_id, resource.type_id);

            u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            u32 set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            u32 size = static_cast<u32>(compiler.get_declared_struct_size(buffer_type));

            std::vector<ReflectedCode::UniformBuffer>& uniform_buffers = current_result_module.m_uniform_buffers;
            uniform_buffers[j].binding = binding;
            uniform_buffers[j].set = set;
            uniform_buffers[j].name = name;
            uniform_buffers[j].size = size;

            JF_ASSERT(base_type.member_types.size() > 0, "this uniform buffer doesn't have any members");
            for (u32 jj = 0; jj < base_type.member_types.size(); jj++) {
                const spirv_cross::SPIRType& member_type = compiler.get_type(base_type.member_types[jj]);
                const std::string&           member_name = compiler.get_member_name(resource.base_type_id, jj);
                u32 member_size = static_cast<u32>(compiler.get_declared_struct_member_size(buffer_type, jj));
                u32 member_offset = static_cast<u32>(compiler.type_struct_member_offset(buffer_type, jj));


                ReflectedCode::UniformBuffer::Member member = {};
                member.name = member_name;
                member.size = member_size;
                member.offset = member_offset;
                member.type = to_SHADER_TYPE(member_type, member_type.vecsize, member_type.columns);
                Logger::info(
                    "\tthe member {}.{} has, type {}, size {} and offset {}", name, member_name, to_string(member.type),
                    member_size, member_offset);

                uniform_buffers[j].members.push_back(member);
            }
            compiler.get_member_name(resource.base_type_id, 0);
            compiler.get_declared_struct_size(compiler.get_type(resource.base_type_id));
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
            std::vector<ReflectedCode::SampledImage>& sampled_images = current_result_module.m_sampled_images;
            sampled_images[j].binding = binding;
            sampled_images[j].set = set;
            sampled_images[j].name = name;
        }
        std::sort(current_result_module.m_inputs.begin(), current_result_module.m_inputs.end(), temp_cmp_0);
        // std::sort(current_result_module.m_outputs.begin(), current_result_module.m_outputs.end(), temp_cmp);



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