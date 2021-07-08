#pragma once

#include <JadeFrame/defines.h>
#include <vector>


namespace JadeFrame {

__declspec(noinline) auto string_to_SPIRV(const char* code, u32 i) -> std::vector<u32>;

}