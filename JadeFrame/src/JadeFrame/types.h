#pragma once
#include <cstdint>

/*
    This file defines custom types for JadeFrame.
    The standard C++ types are too messy and too long, while the ones defines in 'std' are still too long.
    With this file we have a standardized style across the whole project, hopefully making things easier on the eye.
*/

namespace JadeFrame {

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using i8 = int8;
using i16 = int16;
using i32 = int32;
using i64 = int64;

using u8 = uint8;
using u16 = uint16;
using u32 = uint32;
using u64 = uint64;

using f32 = float;
using f64 = double;

constexpr auto operator"" _u8(unsigned long long x) -> u8 { return static_cast<u8>(x); }

constexpr auto operator"" _u16(unsigned long long x) -> u16 { return static_cast<u16>(x); }

constexpr auto operator"" _u32(unsigned long long x) -> u32 { return static_cast<u32>(x); }

constexpr auto operator"" _u64(unsigned long long x) -> u64 { return static_cast<u64>(x); }

constexpr auto operator"" _i8(unsigned long long x) -> i8 { return static_cast<i8>(x); }

constexpr auto operator"" _i16(unsigned long long x) -> i16 { return static_cast<i16>(x); }

constexpr auto operator"" _i32(unsigned long long x) -> i32 { return static_cast<i32>(x); }

constexpr auto operator"" _i64(unsigned long long x) -> i64 { return static_cast<i64>(x); }

constexpr auto operator"" _f32(unsigned long long x) -> f32 { return static_cast<f32>(x); }

constexpr auto operator"" _f64(unsigned long long x) -> f64 { return static_cast<f64>(x); }

constexpr auto operator"" _f32(long double x) -> f32 { return static_cast<f32>(x); }

constexpr auto operator"" _f64(long double x) -> f64 { return static_cast<f64>(x); }

} // namespace JadeFrame