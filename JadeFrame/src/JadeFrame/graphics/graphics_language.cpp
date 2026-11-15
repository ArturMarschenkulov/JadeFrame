#include "graphics_language.h"

// JF_PRAGMA_PUSH
// #pragma warning(disable : 4006)
// #include "shaderc/shaderc.hpp"
// JF_PRAGMA_POP

JF_PRAGMA_NO_WARNINGS_PUSH
#include "shaderc/shaderc.hpp"
#include "SPIRV-Cross/spirv_glsl.hpp"
// #include "SPIRV-Cross/spirv_hlsl.hpp"
// #include "SPIRV-Cross/spirv_msl.hpp"
JF_PRAGMA_NO_WARNINGS_POP

#include <format>

namespace JadeFrame {

namespace {

template<class T>
auto shader_error(ShaderErrorCode code, std::string message) -> Result<T, ShaderError> {
    return err<T, ShaderError>(ShaderError{code, std::move(message)});
}

} // namespace

enum class IMAGE_ADDRESS_MODE : u8 {
    REPEAT,
    MIRRORED_REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER
};
enum class IMAGE_FILTER_MODE : u8 {
    NEAREST,
    LINEAR
};
enum class IMAGE_COMPARE_FUNCTION : u8 {
    LESS,
    LEQUAL,
    GREATER,
    GEQUAL,
    EQUAL,
    NOTEQUAL,
    ALWAYS,
    NEVER
};

auto convert_SPIRV_to_GLSL(const std::vector<u32>& spirv) -> std::string {
    spirv_cross::CompilerGLSL::Options options;
    options.version = 450;
    options.es = false;
    options.vulkan_semantics = false;

    spirv_cross::CompilerGLSL glsl(spirv);
    glsl.set_common_options(options);
    return glsl.compile();
}

// Flattens the descriptor sets of a Vulkan-style module into consecutive bindings and
// returns the resulting GLSL. Throws `spirv_cross::CompilerError` on malformed SPIR-V.
static auto remap_descriptor_sets(const ShadingCode::Module::SPIRV& code)
    -> Result<std::string, ShaderError> {
    namespace spv_c = spirv_cross;

    const auto unsupported_set = [](u32 set) {
        return err<std::string, ShaderError>(ShaderError{
            ShaderErrorCode::INVALID_SPIRV,
            std::format(
                "Descriptor set {} is not supported, only the sets 0 to 3 are", set
            ),
        });
    };

    spv_c::CompilerGLSL compiler = spv_c::CompilerGLSL(code);

    spv_c::CompilerGLSL::Options options;
    options.version = 450;
    options.es = false;
    options.vulkan_semantics = true;
    compiler.set_common_options(options);
    spv_c::ShaderResources resources = compiler.get_shader_resources();

    using Set = std::vector<spv_c::Resource*>;
    std::array<Set, 4> sets;
    for (u32 j = 0; j < resources.uniform_buffers.size(); j++) {
        spv_c::Resource& ub = resources.uniform_buffers[j];

        u32 set = compiler.get_decoration(ub.id, spv::DecorationDescriptorSet);
        if (set >= sets.size()) { return unsupported_set(set); }

        sets[set].push_back(&ub);
    }
    // What would be uniform Sampler2D?
    for (u32 j = 0; j < resources.sampled_images.size(); j++) {
        spv_c::Resource& si = resources.sampled_images[j];

        u32 set = compiler.get_decoration(si.id, spv::DecorationDescriptorSet);
        if (set >= sets.size()) { return unsupported_set(set); }

        sets[set].push_back(&si);
    }

    u32 binding = 0;
    for (u32 i = 0; i < sets.size(); i++) {
        for (u32 j = 0; j < sets[i].size(); j++) {
            spv_c::Resource* ub = sets[i][j];
            compiler.unset_decoration(ub->id, spv::DecorationDescriptorSet);
            // compiler.set_decoration(ub->id, spv::DecorationDescriptorSet, 0);
            compiler.set_decoration(ub->id, spv::DecorationBinding, binding);
            binding++;
        }
    }
    return ok<std::string, ShaderError>(compiler.compile());
}

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
) -> Result<ShadingCode::Module::SPIRV, ShaderError> {
    namespace spv_c = spirv_cross;
    using SPIRV = ShadingCode::Module::SPIRV;

    std::string source;
    // SPIRV-Cross reports malformed modules by throwing. That is an expected failure
    // here, so it is turned into an error value at this boundary.
    try {
        auto remapped = remap_descriptor_sets(code);
        if (remapped.is_err()) {
            return err<SPIRV, ShaderError>(std::move(remapped).error());
        }
        source = std::move(remapped).value();
    } catch (const spv_c::CompilerError& e) {
        return shader_error<SPIRV>(
            ShaderErrorCode::INVALID_SPIRV,
            std::format(
                "Cannot remap the {} shader module for OpenGL: {}",
                to_string(stage),
                e.what()
            )
        );
    }

    if (out_source != nullptr) { *out_source = source; }
    return GLSL_to_SPIRV(source, stage, GRAPHICS_API::OPENGL);
}

// Function to set shader kind based on stage
static auto get_shader_kind(SHADER_STAGE stage, shaderc_shader_kind* out_kind) -> bool {
    switch (stage) {
        case SHADER_STAGE::VERTEX: *out_kind = shaderc_vertex_shader; return true;
        case SHADER_STAGE::FRAGMENT: *out_kind = shaderc_fragment_shader; return true;
        default: return false;
    }
}

// Returns false if there is no compilation target for `api`.
static auto set_compile_options(GRAPHICS_API api, shaderc::CompileOptions& options)
    -> bool {
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
        default: return false;
    }

    options.SetWarningsAsErrors();
    options.SetGenerateDebugInfo();
    const bool optimize = false;
    if constexpr (optimize) {
        options.SetOptimizationLevel(shaderc_optimization_level_size);
    }
    return true;
}

// Translate GLSL to SPIR-V
// Depending on the API, the SPIR-V will be different.
auto GLSL_to_SPIRV(const std::string& glsl_code, SHADER_STAGE stage, GRAPHICS_API api)
    -> Result<std::vector<u32>, ShaderError> {
    using SPIRV = std::vector<u32>;

    shaderc::CompileOptions options;
    if (!set_compile_options(api, options)) {
        return shader_error<SPIRV>(
            ShaderErrorCode::UNSUPPORTED_API,
            std::format("GLSL cannot be compiled for the {} API", to_string(api))
        );
    }
    shaderc_shader_kind kind = {};
    if (!get_shader_kind(stage, &kind)) {
        return shader_error<SPIRV>(
            ShaderErrorCode::UNSUPPORTED_STAGE,
            std::format("{} shaders cannot be compiled", to_string(stage))
        );
    }

    shaderc::Compiler             compiler;
    shaderc::SpvCompilationResult comp_result =
        compiler.CompileGlslToSpv(glsl_code, kind, "", options);
    shaderc_compilation_status comp_status = comp_result.GetCompilationStatus();
    if (comp_status != shaderc_compilation_status_success) {
        return shader_error<SPIRV>(
            ShaderErrorCode::COMPILATION_FAILED,
            std::format(
                "The {} shader does not compile ({} errors):\n{}",
                to_string(stage),
                comp_result.GetNumErrors(),
                comp_result.GetErrorMessage()
            )
        );
    }

    return ok<SPIRV, ShaderError>(comp_result.cbegin(), comp_result.cend());
}
} // namespace JadeFrame
