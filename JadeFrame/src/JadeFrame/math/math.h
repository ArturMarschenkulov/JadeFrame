#pragma once
#undef min
#undef max
#include "JadeFrame/prelude.h"
#include "mat_4.h"
#include "vec.h"
#include <concepts>

// #define M_PI 3.14159265359f // 3.14159265358979323846

namespace JadeFrame {
auto to_radians(f32 degrees) -> f32;
auto to_degrees(f32 radians) -> f32;

auto to_radians(f64 degrees) -> f64;
auto to_degrees(f64 radians) -> f64;

auto to_radians(i32 degrees) -> f32;
auto to_degrees(i32 radians) -> f32;

// // Convert degrees to radians.
// // Accepts float and double, automatically converts integers to float.
// template<typename T = f32>
//     requires std::is_floating_point<T>::value
// auto to_radians(T degrees) -> T {
//     constexpr T DEG_TO_RAD = static_cast<T>(0.01745329251994329576923690768489);
//     return degrees * DEG_TO_RAD;
// }

// // Convert radians to degrees.
// // Accepts float and double, automatically converts integers to float.
// template<typename T = f32>
//     requires std::is_floating_point<T>::value
// auto to_degrees(T radians) -> T {
//     constexpr T RAD_TO_DEG = static_cast<T>(57.295779513082320876798154814105);
//     return radians * RAD_TO_DEG;
// }

template<typename T>
    requires std::integral<T>
auto is_power_of_two(T value) -> bool {
    return ((value & (value - 1)) == (T)0);
}

// floating point bit hackery stuff
inline auto as_u32(f32 value) -> u32 { return *(u32*)&value; }

inline auto as_f32(u32 value) -> f32 { return *(f32*)&value; }

inline auto negate_f32(f32 value) -> f32 { return as_f32(as_u32(value) ^ 0x80000000); }

inline auto log2_f32(f32 value) -> f32 {
    u32 x = as_u32(value);
    u32 y = x >> 23;
    y = y - 127;
    return (f32)y;
}

} // namespace JadeFrame