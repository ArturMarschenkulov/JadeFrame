#pragma once
#include "JadeFrame/defines.h"
#include<string>

namespace JadeFrame {
auto from_wstring_to_string(const std::wstring& wstr)->std::string;
auto from_string_to_wstring(const std::string& str)->std::wstring;

auto custom_simple_hash_0(const std::string& str)->u32;
//constexpr auto custom_simple_hash_1(const char* str) -> u32 {
//	u32 hash = 0;
//	for (u32 i = 0; str[i] != '\0'; i++) {
//		hash = 37 * hash + 17 * str[i];
//	}
//	return hash;
//}

auto get_random_number(i32 begin, i32 end)->i32;
auto map_range(f64 x, f64 in_min, f64 in_max, f64 out_min, f64 out_max)->f64;
}