#pragma once
#include <type_traits>
#include <concepts>
#undef min
#undef max
#include "JadeFrame/prelude.h"


// #define M_PI 3.14159265359f // 3.14159265358979323846

namespace JadeFrame {
constexpr auto to_radians(f32 degrees) -> f32 {
    // return degrees * (M_PI / 180.0f);
    constexpr f32 DEG_TO_RAD = 0.01745329251994329576923690768489F;
    return degrees * DEG_TO_RAD;
}

constexpr auto to_degrees(f32 radians) -> f32 {
    // return radians * (180.0f / M_PI);
    constexpr f32 RAD_TO_DEG = 57.295779513082320876798154814105F;
    return radians * RAD_TO_DEG;
}

constexpr auto to_radians(f64 degrees) -> f64 {
    // return degrees * (M_PI / 180.0f);
    constexpr f32 DEG_TO_RAD = 0.01745329251994329576923690768489F;
    return degrees * DEG_TO_RAD;
}

constexpr auto to_degrees(f64 radians) -> f64 {
    // return radians * (180.0f / M_PI);
    constexpr f32 RAD_TO_DEG = 57.295779513082320876798154814105F;
    return radians * RAD_TO_DEG;
}

constexpr auto to_radians(i32 degrees) -> f32 { return to_radians((f32)degrees); }

constexpr auto to_degrees(i32 radians) -> f32 { return to_degrees((f32)radians); }

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

// floating point bit hackery stuff
inline auto as_u32(f32 value) -> u32 { return *(u32*)&value; }

inline auto as_f32(u32 value) -> f32 { return *(f32*)&value; }

inline auto negate_f32(f32 value) -> f32 { return as_f32(as_u32(value) ^ 0x80000000); }

namespace math {

/**
 * Calculates the aligned block size based on the given block size and alignment.
 *
 * @param block_size The size of the block.
 * @param alignment The desired alignment.
 * @return The aligned block size.
 *
 * @example
 * ceil_to_multiple(20, 16) == 32
 * ceil_to_multiple(32, 32) == 32
 * ceil_to_multiple(13, 16) == 16
 * ceil_to_multiple(15, 16) == 16
 * ceil_to_multiple(16, 16) == 16
 * ceil_to_multiple(17, 16) == 32
 */
static constexpr auto ceil_to_aligned(const u64 value, const u64 alignment) -> u64 {
#if 1 // more efficient
    const u64 new_val = (value + alignment - 1) & ~(alignment - 1);
    const u64 aligned_block_size = alignment > 0 ? new_val : value;
#else // more readable
    const u64 new_val = (value + alignment - (value % alignment));
    const u64 aligned_block_size = (value % alignment == 0) ? value : new_val;
#endif
    return aligned_block_size;
}

constexpr auto pi() -> f64 { return 3.141592653589793238462643383279502884; }

constexpr auto factorial(u32 n) -> f64 { return (n <= 1) ? 1.0 : (n * factorial(n - 1)); }

constexpr auto power(f64 base, u32 exp) -> f64 {
    return (exp == 0) ? 1.0 : base * power(base, exp - 1);
}

constexpr auto sin_constexpr(f32 x, u32 terms = 10) -> f64 {
    f64 result = 0.0;
    for (u32 n = 0; n < terms; ++n) {
        f64 term = power(-1.0, n) * power(x, 2 * n + 1) / factorial(2 * n + 1);
        result += term;
    }
    return result;
}

constexpr auto cos_constexpr(f64 x, u32 terms = 10) -> f64 {
    f64 result = 0.0;
    for (u32 n = 0; n < terms; ++n) {
        f64 term = power(-1.0, n) * power(x, 2 * n) / factorial(2 * n);
        result += term;
    }
    return result;
}

// constexpr auto atan_constexpr(f64 x, u32 terms = 10) -> f64 {
//     if (x > 1.0) {
//         return (3.14159265358979323846 / 2) - atan_constexpr(1.0 / x, terms);
//     } else if (x < -1.0) {
//         return -(3.14159265358979323846 / 2) - atan_constexpr(1.0 / x, terms);
//     }

//     f64 result = 0.0;
//     for (int n = 0; n < terms; ++n) {
//         f64 term = power(-1.0, n) * power(x, 2 * n + 1) / (2 * n + 1);
//         result += term;
//     }
//     return result;
// }

// constexpr auto atan(f32 x, u32 terms = 10) -> f32 {
//     if (std::is_constant_evaluated()) {
//         return static_cast<f32>(atan_constexpr(x, terms));
//     } else {
//         return std::atan(x);
//     }
// }

constexpr auto tan_constexpr(f32 x, u32 terms = 10) -> f64 {
    return sin_constexpr(x, terms) / cos_constexpr(x, terms);
}

constexpr auto tan(f32 x, u32 terms = 10) -> f32 {
    if (std::is_constant_evaluated()) {
        return static_cast<f32>(tan_constexpr(x, terms));
    } else {
        return std::tan(x);
    }
}

constexpr auto sin(f32 x, u32 terms = 14) -> f32 {
    if (std::is_constant_evaluated()) {
        return static_cast<f32>(sin_constexpr(x, terms));
    } else {
        return std::sin(x);
    }
}

constexpr auto cos(f32 x, u32 terms = 15) -> f32 {
    if (std::is_constant_evaluated()) {
        return static_cast<f32>(cos_constexpr(x, terms));
    } else {
        return std::cos(x);
    }
}

template<typename T>
    requires std::integral<T>
auto is_power_of_two(T value) -> bool {
    return ((value & (value - 1)) == (T)0);
}

inline auto log2_f32(f32 value) -> f32 {
    u32 x = as_u32(value);
    u32 y = x >> 23;
    y = y - 127;
    return (f32)y;
}
} // namespace math

} // namespace JadeFrame