#pragma once
#include <string>

#include "graphics_shared.h"

namespace JadeFrame {

class GLSLCodeLoader {
public:
    /// Compiles the built-in shader registered under `name` to SPIR-V.
    ///
    /// Fails with `UNKNOWN_SHADER_NAME` if there is no such shader. Failures are
    /// returned and not logged, reporting them is up to the caller.
    [[nodiscard]] static auto get_by_name(const std::string& name)
        -> Result<ShadingCode, ShaderError>;
};

} // namespace JadeFrame