#include "graphics_language.h"
#include "JadeFrame/utils/assert.h"

// JF_PRAGMA_PUSH
// #pragma warning(disable : 4006)
// #include "shaderc/shaderc.hpp"
// JF_PRAGMA_POP

JF_PRAGMA_NO_WARNINGS_PUSH
#include "shaderc/shaderc.hpp"
#include "SPIRV-Cross/spirv_glsl.hpp"
#include "SPIRV-Cross/spirv_hlsl.hpp"
#include "SPIRV-Cross/spirv_msl.hpp"
JF_PRAGMA_NO_WARNINGS_POP

namespace JadeFrame {

// Makes the SPIRV code compatible with opengl.
// The SPIRV of Vulkan and SPIRV of OpenGL are slightly different.
//
// The whole issue comes from the fact that the GLSL code which this graphics framework
// uses has to be written in Vulkan-style GLSL. This is because Vulkan style GLSL can be
// converted to OpenGL-style GLSL without much problems. However, the other way around is
// more difficult.
//
// Thus the workflow is as follows, when using OpenGL:
// 1. Write GLSL code in Vulkan-style GLSL
// 2. Convert Vulkan-style GLSL to SPIRV
// 3. Convert Vulkan-style SPIRV to more OpenGL-style SPIRV
// 4. Convert OpenGL-style SPIRV to OpenGL-style GLSL
// 5. Compile OpenGL-style GLSL to OpenGL-style SPIRV (optionally or rather one skips the
// previous step).
//
// Step 3 is highly variable and will be modified in the future to better compatiblity.
auto remap_for_opengl(
    const ShadingCode::Module::SPIRV& code,
    SHADER_STAGE                      stage,
    std::string*                      out_source
) -> ShadingCode::Module::SPIRV {
    // alias spirv_cross with spv_c
    namespace spv_c = spirv_cross;

    spv_c::CompilerGLSL compiler = spv_c::CompilerGLSL(code);
    // sets up opengl compiler options
    spv_c::CompilerGLSL::Options options;
    options.version = 450;
    options.es = false;
    options.vulkan_semantics = true;
    compiler.set_common_options(options);
    spv_c::ShaderResources resources = compiler.get_shader_resources();

    std::array<std::vector<spv_c::Resource*>, 4> uniform_buffer_sets;
    for (u32 j = 0; j < resources.uniform_buffers.size(); j++) {
        spv_c::Resource& uniform_buffer = resources.uniform_buffers[j];

        u32 set =
            compiler.get_decoration(uniform_buffer.id, spv::DecorationDescriptorSet);
        u32 binding = compiler.get_decoration(uniform_buffer.id, spv::DecorationBinding);
        JF_ASSERT(
            set <= 3,
            "As of right now, only 4 descriptor sets are supported. (0, 1, 2, 3)"
        );

        uniform_buffer_sets[set].push_back(&uniform_buffer);
    }
    // What would be uniform Sampler2D?
    for (u32 j = 0; j < resources.sampled_images.size(); j++) {
        spv_c::Resource& resource = resources.sampled_images[j];

        u32 set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        JF_ASSERT(
            set <= 3,
            "As of right now, only 4 descriptor sets are supported. (0, 1, 2, 3)"
        );

        uniform_buffer_sets[set].push_back(&resource);
    }

    u32 binding = 0;
    for (u32 i = 0; i < uniform_buffer_sets.size(); i++) {
        for (u32 j = 0; j < uniform_buffer_sets[i].size(); j++) {
            compiler.unset_decoration(
                uniform_buffer_sets[i][j]->id, spv::DecorationDescriptorSet
            );
            // compiler.set_decoration(dd[i][j]->id, spv::DecorationDescriptorSet, 0);
            compiler.set_decoration(
                uniform_buffer_sets[i][j]->id, spv::DecorationBinding, binding
            );
            binding++;
        }
    }
    auto source = compiler.compile();
    *out_source = source;
    return GLSL_to_SPIRV(source, stage, GRAPHICS_API::OPENGL);
}

// Function to set shader kind based on stage
static auto get_shader_kind(SHADER_STAGE stage) -> shaderc_shader_kind {
    switch (stage) {
        case SHADER_STAGE::VERTEX: return shaderc_vertex_shader;
        case SHADER_STAGE::FRAGMENT: return shaderc_fragment_shader;
        default: assert(false); return {}; // Or return some default kind
    }
}

// Translate GLSL to SPIR-V
// Depending on the API, the SPIR-V will be different.
auto GLSL_to_SPIRV(const std::string& glsl_code, SHADER_STAGE stage, GRAPHICS_API api)
    -> std::vector<u32> {

    shaderc::CompileOptions options;
    switch (api) {
        case GRAPHICS_API::VULKAN: {
            options.SetTargetEnvironment(
                shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2
            );
        } break;
        case GRAPHICS_API::OPENGL: {
            options.SetTargetEnvironment(
                shaderc_target_env_opengl, shaderc_env_version_opengl_4_5
            );
        } break;
        default: JF_UNIMPLEMENTED("");
    }
    options.SetWarningsAsErrors();
    options.SetGenerateDebugInfo();
    const bool optimize = false;
    if constexpr (optimize) {
        options.SetOptimizationLevel(shaderc_optimization_level_size);
    }
    shaderc_shader_kind kind = get_shader_kind(stage);

    shaderc::Compiler             compiler;
    shaderc::SpvCompilationResult comp_result =
        compiler.CompileGlslToSpv(glsl_code, kind, "", options);
    shaderc_compilation_status comp_status = comp_result.GetCompilationStatus();
    if (comp_status != shaderc_compilation_status_success) {
        assert(false);
        return std::vector<u32>();
    }

    std::vector<u32> result = {comp_result.cbegin(), comp_result.cend()};
    return result;
}
} // namespace JadeFrame