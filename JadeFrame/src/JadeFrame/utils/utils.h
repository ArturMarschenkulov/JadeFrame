#pragma once
#include <string>
#include <bitset>
#include <span>
#include <memory>

#include "JadeFrame/types.h"

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

auto from_kibibyte(u64 value) -> u64;
auto from_mebibyte(u64 value) -> u64;
auto from_gibibyte(u64 value) -> u64;
auto from_tebibyte(u64 value) -> u64;

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
}

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
auto make_ref(T* obj) -> Ref<T> {
    return std::make_shared<T>(obj);
}

template<class T>
constexpr auto take_and_destruct(T& t) noexcept -> T {
    auto taken = T(static_cast<T&&>(t));
    t.~T();
    return taken;
}

template<typename T>
constexpr auto take_copy_and_destruct(T& value) -> T {
    T temp = value;
    value.~T();
    return temp;
}

namespace bit {

template<typename T>
auto lshift(T value, u32 shift) -> T {
    return value << shift;
}

template<typename T, typename U>
auto set(T value, u32 bit) -> T {
    return value | (1 << bit);
}

template<typename T, typename U>
auto unset(T value, u32 bit) -> T {
    return value & ~(1 << bit);
}

template<typename T>
auto toggle(T value, u32 bit) -> T {
    return value ^ (1 << bit);
}

template<typename T>
auto check(T value, u32 bit) -> bool {
    return value & (1 << bit);
}

template<typename T>
auto set_flag(T value, u32 flag) -> T {
    return value | flag;
}

template<typename T>
auto set_flags(const T value, std::span<u32> flags) -> T {
    T result = value;
    for (const u32 flag : flags) { result |= flag; }
    return result;
}

inline auto set_flags(std::span<u32> flags) -> u32 {
    u32 result = 0;
    for (const u32 flag : flags) { result |= flag; }
    return result;
}

template<typename T>
auto unset_flag(T value, u32 flag) -> T {
    return value & ~flag;
}

template<typename T>
auto toggle_flag(T value, u32 flag) -> T {
    return value ^ flag;
}

template<typename T>
auto flag(u32 flag) -> bool {
    return 0U | flag;
}

template<typename T>
auto check_flag(T value, u32 flag) -> bool {
    return value & flag;
}

template<typename T>
auto count_ones(T value) -> u32 {
    return std::bitset<sizeof(T) * 8>(value).count();
}

template<typename T>
auto to_byte_array(const T& value) -> std::array<u8, sizeof(T)> {
    std::array<u8, sizeof(T)> bytes;
    std::memcpy(bytes.data(), &value, sizeof(T));
    return bytes;
}

template<typename T>
auto to_bitset(const T& value) -> std::bitset<sizeof(T) * 8> {
    return std::bitset<sizeof(T) * 8>(value);
}

} // namespace bit

} // namespace JadeFrame