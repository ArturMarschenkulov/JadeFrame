#include "graphics_shared.h"
#include "JadeFrame/prelude.h"

namespace JadeFrame {
auto GLSL_to_SPIRV(const std::string& code, SHADER_STAGE stage, GRAPHICS_API api)
    -> std::vector<u32>;
auto remap_for_opengl(
    const ShadingCode::Module::SPIRV& code,
    SHADER_STAGE                      stage,
    std::string*                      out_source
) -> ShadingCode::Module::SPIRV;
auto convert_SPIRV_to_GLSL(const std::vector<u32>& spirv) -> std::string;

} // namespace JadeFrame