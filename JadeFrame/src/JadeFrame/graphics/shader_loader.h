#pragma once
#include "graphics_shared.h"
#include <string>
#include <tuple>

namespace JadeFrame {


class GLSLCodeLoader {
public:
    static auto get_by_name(const std::string& name) -> ShadingCode;
};

} // namespace JadeFrame