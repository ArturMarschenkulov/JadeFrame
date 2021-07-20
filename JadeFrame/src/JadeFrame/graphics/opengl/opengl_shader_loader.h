#pragma once
#include <tuple>
#include <string>

namespace JadeFrame {

struct GLSLCode {
	std::string m_vertex_shader;
	std::string m_fragment_shader;
};
class GLSLCodeLoader {
public:
	static auto get_by_name(const std::string& name) -> GLSLCode;
};
auto get_glsl_code_by_name(const std::string& name)->std::tuple<std::string, std::string>;
}