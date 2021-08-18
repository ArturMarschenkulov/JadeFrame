#include "pch.h"
#include "to_spirv.h"

#pragma warning(push)
#pragma warning(disable:4006)
#include "extern/shaderc/shaderc.hpp"
#pragma warning(pop)

namespace JadeFrame {

__declspec(noinline) auto string_to_SPIRV(const char* code, u32 i) -> std::vector<u32> {
	using namespace shaderc;
	Compiler compiler;
	CompileOptions options;
	options.SetOptimizationLevel(shaderc_optimization_level_size);
	SpvCompilationResult result = compiler.CompileGlslToSpv(std::string(code), (shaderc_shader_kind)i /*shaderc_vertex_shader*/, "", options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		__debugbreak();
		return std::vector<u32>();
	}

	std::vector<u32> res = { result.cbegin(), result.cend() };
	return res;
}

}