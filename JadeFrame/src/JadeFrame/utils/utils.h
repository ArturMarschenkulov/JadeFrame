#pragma once
#include "JadeFrame/prelude.h"
#include <string>

namespace JadeFrame {
auto custom_simple_hash_0(const std::string& str) -> u32;
// constexpr auto custom_simple_hash_1(const char* str) -> u32 {
//	u32 hash = 0;
//	for (u32 i = 0; str[i] != '\0'; i++) {
//		hash = 37 * hash + 17 * str[i];
//	}
//	return hash;
// }

auto get_random_number(i32 begin, i32 end) -> i32;
auto map_range(f64 x, f64 in_min, f64 in_max, f64 out_min, f64 out_max) -> f64;

inline auto from_kibibyte(u64 value) -> u64 { return value * 1024; }
inline auto from_mebibyte(u64 value) -> u64 { return from_kibibyte(value) * 1024; }
inline auto from_gibibyte(u64 value) -> u64 { return from_mebibyte(value) * 1024; }
inline auto from_tebibyte(u64 value) -> u64 { return from_gibibyte(value) * 1024; }

// auto byte_to_kibi(u64 value) -> u64 {
//	return value * 1024;
// }
// auto byte_to_mebi(u64 value) -> u64 {
//	return kibi_to_byte(value) * 1024;
// }
// auto byte_to_gibi(u64 value)-> u64 {
//	return mebi_to_byte(value) * 1024;
// }
// auto byte_to_gibi(u64 value) -> u64 {
//	return gibi_to_byte(value) * 1024;
// }

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T>
auto make_scope(T* obj) -> Scope<T> {
    return std::make_unique<T>(obj);
};

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T>
auto make_ref(T* obj) -> Ref<T> {
    return std::make_shared<T>(obj);
};

} // namespace JadeFrame