#include "reflect.h"
#include <set>
#include "graphics_language.h"

#include "JadeFrame/utils/assert.h"

JF_PRAGMA_NO_WARNINGS_PUSH
#include "SPIRV-Cross/spirv_glsl.hpp"
// #include "SPIRV-Cross/spirv_hlsl.hpp"
// #include "SPIRV-Cross/spirv_msl.hpp"
JF_PRAGMA_NO_WARNINGS_POP

namespace JadeFrame {

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

static auto to_SHADER_TYPE(const spirv_cross::SPIRType& type, u32 rows, u32 columns)
    -> SHADER_TYPE {
    SHADER_TYPE result = SHADER_TYPE::NONE;
    if (rows > 4) { JF_PANIC("rows must be less than 4"); }
    if (columns > 4) { JF_PANIC("columns must be less than 4"); }
    if (rows == 0 || columns == 0) { JF_PANIC("Invalid vector/matrix dimensions"); }

    if (columns == 1) {
        switch (type.basetype) {
            case spirv_cross::SPIRType::Float: {
                u32         index = rows - 1;
                SHADER_TYPE arr[] = {
                    SHADER_TYPE::F32,
                    SHADER_TYPE::V_2_F32,
                    SHADER_TYPE::V_3_F32,
                    SHADER_TYPE::V_4_F32
                };
                result = arr[index];
            } break;
                JF_UNIMPLEMENTED(
                    "only float basetype is supported in to_SHADER_TYPE for now"
                );
        }
    } else {
        switch (type.basetype) {
            case spirv_cross::SPIRType::Float: {
                if (columns != rows) {
                    Logger::err(
                        "matrix types with different row and column count "
                        "are not supported yet! {} {}",
                        rows,
                        columns
                    );
                    JF_UNIMPLEMENTED(
                        "matrix types with different row and column count "
                        "are not supported yet!"
                    );
                }
                SHADER_TYPE arr[] = {
                    SHADER_TYPE::M_2_2_F32, SHADER_TYPE::M_3_3_F32, SHADER_TYPE::M_4_4_F32
                };

                result = arr[rows - 2];
            } break;
                JF_UNIMPLEMENTED(
                    "only float basetype is supported in to_SHADER_TYPE for now"
                );
        }
    }
    return result;
}

auto temp_cmp_1(const ReflectedModule::Input& i0, const ReflectedModule::Input& i1)
    -> bool {
    return i0.location < i1.location;
}

auto temp_cmp(const ReflectedModule::Output& i0, const ReflectedModule::Output& i1)
    -> bool {
    return i0.location < i1.location;
}

static auto reflect_inputs(
    spirv_cross::Compiler&                                 comp,
    const spirv_cross::SmallVector<spirv_cross::Resource>& stage_inputs
) -> std::vector<ReflectedModule::Input> {
    constexpr u32 BYTE_SIZE = 8;

    std::vector<ReflectedModule::Input> result;

    result.resize(stage_inputs.size());
    for (u32 j = 0; j < stage_inputs.size(); j++) {
        const spirv_cross::Resource& rc = stage_inputs[j];
        const std::string&           name = rc.name;
        const spirv_cross::SPIRType& base_ty = comp.get_type(rc.base_type_id);
        const spirv_cross::SPIRType& buf_ty = comp.get_type(rc.type_id);

        u32 member_count = static_cast<u32>(buf_ty.member_types.size());
        u32 location = comp.get_decoration(rc.id, spv::DecorationLocation);
        u32 size = (buf_ty.width / BYTE_SIZE) * buf_ty.vecsize * buf_ty.columns;

        result[j].name = name;
        result[j].location = location;
        result[j].size = size;
        result[j].type = to_SHADER_TYPE(buf_ty, buf_ty.vecsize, buf_ty.columns);
    }
    return result;
}

static auto reflect_outputs(
    spirv_cross::Compiler&                                 comp,
    const spirv_cross::SmallVector<spirv_cross::Resource>& stage_outputs
) -> std::vector<ReflectedModule::Output> {
    constexpr u32 BITS_PER_BYTE = 8;

    std::vector<ReflectedModule::Output> result;

    result.resize(stage_outputs.size());
    for (u32 j = 0; j < stage_outputs.size(); j++) {
        const spirv_cross::Resource& rc = stage_outputs[j];
        const std::string&           name = rc.name;
        const spirv_cross::SPIRType& base_ty = comp.get_type(rc.base_type_id);
        const spirv_cross::SPIRType& buf_ty = comp.get_type(rc.type_id);

        u32 mem_count = static_cast<u32>(buf_ty.member_types.size());
        u32 location = comp.get_decoration(rc.id, spv::DecorationLocation);
        u32 size = (buf_ty.width / BITS_PER_BYTE) * buf_ty.vecsize * buf_ty.columns;

        result[j].name = name;
        result[j].location = location;
        result[j].size = size;
        result[j].type = to_SHADER_TYPE(buf_ty, buf_ty.vecsize, buf_ty.columns);
    }
    return result;
}

static auto reflect_uniforms(
    spirv_cross::Compiler&                                 comp,
    const spirv_cross::SmallVector<spirv_cross::Resource>& uniform_buffers
) -> std::vector<ReflectedModule::UniformBuffer> {
    constexpr u32                               BYTE_SIZE = 8;
    std::vector<ReflectedModule::UniformBuffer> result;
    result.resize(uniform_buffers.size());
    for (u32 j = 0; j < uniform_buffers.size(); j++) {
        const spirv_cross::Resource& rc = uniform_buffers[j];

        const std::string&           name = rc.name;
        const spirv_cross::SPIRType& buf_ty = comp.get_type(rc.type_id);
        const spirv_cross::SPIRType& base_ty = comp.get_type(rc.base_type_id);

        Logger::info(
            "the uniform buffer {} has {} members. base id {}, id {}",
            name,
            buf_ty.member_types.size(),
            rc.base_type_id,
            rc.type_id
        );

        u32 binding = comp.get_decoration(rc.id, spv::DecorationBinding);
        u32 set = comp.get_decoration(rc.id, spv::DecorationDescriptorSet);
        u32 size = static_cast<u32>(comp.get_declared_struct_size(buf_ty));

        result[j].binding = binding;
        result[j].set = set;
        result[j].name = name;
        result[j].size = size;

        JF_ASSERT(
            !base_ty.member_types.empty(), "this uniform buffer doesn't have any members"
        );
        for (u32 jj = 0; jj < base_ty.member_types.size(); jj++) {
            const spirv_cross::SPIRType& mem_ty = comp.get_type(base_ty.member_types[jj]);
            const std::string& mem_name = comp.get_member_name(rc.base_type_id, jj);
            u32                mem_size =
                static_cast<u32>(comp.get_declared_struct_member_size(buf_ty, jj));
            u32 mem_offset = static_cast<u32>(comp.type_struct_member_offset(buf_ty, jj));

            ReflectedModule::UniformBuffer::Member mem = {};
            mem.name = mem_name;
            mem.size = mem_size;
            mem.offset = mem_offset;
            mem.type = to_SHADER_TYPE(mem_ty, mem_ty.vecsize, mem_ty.columns);
            Logger::info(
                "\tthe member {}.{} has, type {}, size {} and offset {}",
                name,
                mem_name,
                to_string(mem.type),
                mem_size,
                mem_offset
            );

            result[j].members.push_back(mem);
        }
        comp.get_member_name(rc.base_type_id, 0);
        comp.get_declared_struct_size(comp.get_type(rc.base_type_id));
    }
    return result;
}

static auto reflect_sampled_images(
    spirv_cross::Compiler&                                 comp,
    const spirv_cross::SmallVector<spirv_cross::Resource>& sampled_images
) -> std::vector<ReflectedModule::SampledImage> {
    std::vector<ReflectedModule::SampledImage> result;
    result.resize(sampled_images.size());
    for (u32 j = 0; j < sampled_images.size(); j++) {
        const spirv_cross::Resource& rc = sampled_images[j];

        const std::string&           name = rc.name;
        const spirv_cross::SPIRType& base_ty = comp.get_type(rc.base_type_id);
        const spirv_cross::SPIRType& buf_ty = comp.get_type(rc.type_id);
        u32 member_count = static_cast<u32>(buf_ty.member_types.size());
        u32 binding = comp.get_decoration(rc.id, spv::DecorationBinding);
        u32 set = comp.get_decoration(rc.id, spv::DecorationDescriptorSet);
        u32 descriptor_set = comp.get_decoration(rc.id, spv::DecorationDescriptorSet);

        u32 dimension = base_ty.image.dim;
        u32 array_size = buf_ty.array[0];

        if (array_size == 0) {
            array_size = 1;
        } else {
            assert(false);
        }
        result[j].binding = binding;
        result[j].set = set;
        result[j].name = name;
    }
    return result;
}

auto ReflectedModule::reflect(const ShadingCode::Module::SPIRV& code, SHADER_STAGE stage)
    -> ReflectedModule {
    ReflectedModule result = {};
    result.m_stage = stage;

    constexpr u32 BYTE_SIZE = 8;

    spirv_cross::Compiler        compiler(code);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    debug_print_resources(resources);

    result.m_inputs = reflect_inputs(compiler, resources.stage_inputs);
    result.m_outputs = reflect_outputs(compiler, resources.stage_outputs);
    result.m_uniform_buffers = reflect_uniforms(compiler, resources.uniform_buffers);
    result.m_sampled_images = reflect_sampled_images(compiler, resources.sampled_images);

    std::sort(result.m_inputs.begin(), result.m_inputs.end(), temp_cmp_1);
    // std::sort(result.m_outputs.begin(), result.m_outputs.end(), temp_cmp);

    // result.m_push_constant_ranges.resize(resources.push_constant_buffers.size());
    // for (u32 j = 0; j < resources.push_constant_buffers.size(); j++) {
    //     const spirv_cross::Resource& resource = resources.push_constant_buffers[j];

    //     const std::string&           buffer_name = resource.name;
    //     const spirv_cross::SPIRType& buffer_type =
    //     compiler.get_type(resource.base_type_id); u32 buffer_size =
    //     (u32)compiler.get_declared_struct_size(buffer_type); u32 member_count =
    //     uint32_t(buffer_type.member_types.size()); u32 buffer_offset = 0;

    //     std::vector<VkPushConstantRange>& push_constant_ranges =
    //     result.m_push_constant_ranges; push_constant_ranges[j].stageFlags =
    //     VK_SHADER_STAGE_VERTEX_BIT; push_constant_ranges[j].size = buffer_size -
    //     buffer_offset; push_constant_ranges[j].offset = buffer_offset;
    // }
    return result;
}

// Takes in modules and returns a single module which is the interface to the whole
// pipeline, that is only the actually interactible points are exposed.
auto ReflectedModule::into_interface(const std::span<const ReflectedModule>& modules)
    -> ReflectedModule {
    ReflectedModule                result = {};
    std::set<u32>                  input_locs;
    std::set<u32>                  output_locs;
    std::set<std::tuple<u32, u32>> uniform_locs;

    for (size_t i = 0; i < modules.size(); i++) {
        const auto& mod = modules[i];

        for (size_t j = 0; j < mod.m_inputs.size(); j++) {
            const auto& input = mod.m_inputs[j];
            if (!input_locs.contains(input.location)) {
                result.m_inputs.push_back(input);
                input_locs.insert(input.location);
            }
        }

        for (int j = (int)mod.m_outputs.size() - 1; j >= 0; j--) {
            const auto& output = mod.m_outputs[j];
            if (!output_locs.contains(output.location)) {
                result.m_outputs.push_back(output);
                output_locs.insert(output.location);
            }
        }

        for (size_t j = 0; j < mod.m_uniform_buffers.size(); j++) {
            const auto& uniform = mod.m_uniform_buffers[j];
            if (!uniform_locs.contains({uniform.set, uniform.binding})) {
                result.m_uniform_buffers.push_back(uniform);
                uniform_locs.insert({uniform.set, uniform.binding});
            } else {
                // This means that the same set and binding is used in another module.
                // TODO: As of right now we won't do anything about it, however in the
                // future we will create extra requirements so that one can't make dumb
                // mistakes.
            }
        }
    }

    return result;
}

auto ReflectedModule::get_vertex_format() const -> VertexFormat {
    std::vector<VertexAttribute> list;
    for (u32 i = 0; i < m_inputs.size(); i++) {
        const Input& input = m_inputs[i];
        list.emplace_back(input.type);
    }
    return VertexFormat{list};
}
} // namespace JadeFrame