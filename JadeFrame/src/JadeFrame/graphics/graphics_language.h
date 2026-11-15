#pragma once
#include "graphics_shared.h"
#include "JadeFrame/prelude.h"

namespace JadeFrame {

/// Compiles GLSL to SPIR-V for the given API. A rejected source yields
/// `COMPILATION_FAILED` with the compiler's diagnostics as message.
[[nodiscard]] auto
GLSL_to_SPIRV(const std::string& code, SHADER_STAGE stage, GRAPHICS_API api)
    -> Result<std::vector<u32>, ShaderError>;

/// Rewrites Vulkan-style SPIR-V so that OpenGL accepts it. Fails with `INVALID_SPIRV`
/// if the module is malformed or uses a descriptor set other than 0 to 3.
[[nodiscard]] auto remap_for_opengl(
    const ShadingCode::Module::SPIRV& code,
    SHADER_STAGE                      stage,
    std::string*                      out_source
) -> Result<ShadingCode::Module::SPIRV, ShaderError>;

auto convert_SPIRV_to_GLSL(const std::vector<u32>& spirv) -> std::string;

} // namespace JadeFrame
