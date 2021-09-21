#pragma once
#include "../shared.h"
#include <tuple>
#include <string>

namespace JadeFrame {


class GLSLCodeLoader {
public:
	static auto get_by_name(const std::string& name) -> ShadingCode;
};

}