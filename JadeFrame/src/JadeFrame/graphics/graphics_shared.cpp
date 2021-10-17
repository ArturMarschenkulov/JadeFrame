#include "pch.h"
#include "graphics_shared.h"
#pragma warning(push)
#pragma warning(disable:4006)
#include "extern/shaderc/shaderc.hpp"
#pragma warning(pop)
namespace JadeFrame {

VertexAttribute::VertexAttribute(const std::string& name, SHADER_TYPE type,  bool normalized)
	: name(name)
	, type(type)
	, size(SHADER_TYPE_get_size(type))
	, offset(0)
	, normalized(normalized) {
}

VertexFormat::VertexFormat(const std::initializer_list<VertexAttribute>& attributes)
	: m_attributes(attributes) {
	this->calculate_offset_and_stride(m_attributes);
}
auto VertexFormat::calculate_offset_and_stride(std::vector<VertexAttribute>& attributes) -> void {
	size_t offset = 0;
	m_stride = 0;
	for (VertexAttribute& attribute : attributes) {
		attribute.offset = offset;
		offset += attribute.size;
		m_stride += attribute.size;
	}
}



auto string_to_SPIRV(const std::string& code, SHADER_STAGE stage) -> std::vector<u32> {
	namespace shc = shaderc;
	//using namespace shaderc;

	shaderc_shader_kind kind;
	switch(stage) {
		case SHADER_STAGE::VERTEX:
		{
			kind = shaderc_vertex_shader;
		} break;
		case SHADER_STAGE::FRAGMENT:
		{
			kind = shaderc_fragment_shader;
		} break;
		default: __debugbreak();
	}

	shc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
	//options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
	options.SetWarningsAsErrors();
	options.SetGenerateDebugInfo();
	const bool optimize = false;
	if (optimize == true) {
		options.SetOptimizationLevel(shaderc_optimization_level_size);
	}
	shc::Compiler compiler;
	shc::SpvCompilationResult comp_result = compiler.CompileGlslToSpv(code, kind, "", options);
	shaderc_compilation_status comp_status = comp_result.GetCompilationStatus();
	if (comp_status != shaderc_compilation_status_success) {
		__debugbreak();
		return std::vector<u32>();
	}

	std::vector<u32> result = { comp_result.cbegin(), comp_result.cend() };
	return result;
}
}