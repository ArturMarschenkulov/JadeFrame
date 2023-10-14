#pragma once
#undef min
#undef max
#include "JadeFrame/prelude.h"
#include "mat_4.h"
#include "vec.h"

// #define M_PI 3.14159265359f // 3.14159265358979323846

namespace JadeFrame {
auto to_radians(f32 degrees) -> f32;
auto to_degrees(f32 radians) -> f32;

template<typename T>
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