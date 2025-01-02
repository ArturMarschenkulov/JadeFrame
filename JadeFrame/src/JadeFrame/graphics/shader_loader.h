#pragma once
#include <string>
#include <tuple>

#include "graphics_shared.h"

namespace JadeFrame {

class GLSLCodeLoader {
public:
    static auto get_by_name(const std::string& name) -> ShadingCode;
};

} // namespace JadeFrame