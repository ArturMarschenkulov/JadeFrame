#pragma once

#include <JadeFrame/defines.h>
#include <vector>
#include <string>

namespace JadeFrame {

auto string_to_SPIRV(const char* code, u32 i) -> std::vector<u32>;

}