#pragma once
#include <tuple>
#include <string>

auto load_shader_by_name(const std::string& name)->std::tuple<std::string, std::string>;