#include "pch.h"
#include "to_spirv.h"

#pragma warning(push)
#pragma warning(disable:4006)
#include "extern/shaderc/shaderc.hpp"
#pragma warning(pop)

namespace JadeFrame {

auto string_to_SPIRV(const char* code, u32 i) -> std::vector<u32> {
	namespace shc = shaderc;
	//using namespace shaderc;

	shc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
	//options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
	options.SetWarningsAsErrors();
	options.SetGenerateDebugInfo();
	const bool optimize = false;
	if(optimize == true) {
		options.SetOptimizationLevel(shaderc_optimization_level_size);
	}
	shc::Compiler compiler;
	shc::SpvCompilationResult comp_result = compiler.CompileGlslToSpv(std::string(code), (shaderc_shader_kind)i /*shaderc_vertex_shader*/, "", options);
	shaderc_compilation_status comp_status = comp_result.GetCompilationStatus();
	if (comp_status != shaderc_compilation_status_success) {
		__debugbreak();
		return std::vector<u32>();
	}

	std::vector<u32> result = { comp_result.cbegin(), comp_result.cend() };
	return result;
}

}