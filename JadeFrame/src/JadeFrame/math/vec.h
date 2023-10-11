#pragma once
#include "JadeFrame/prelude.h"
// #include <concepts>
#include <cmath>
#include <cstring>
#include <type_traits>

namespace JadeFrame {
JF_PRAGMA_PUSH

#if defined(__clang__)
    #pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
    #pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(__GNUC__)
// #pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
// #pragma GCC diagnostic ignored "-Wnested-anon-types"
#elif defined(_MSC_VER)
    #pragma warning(disable : 4201)
#endif

// template <typename T>
// concept scalar = std::is_scalar_v<T>;
//
// template <typename T>
// concept number = std::integral<T> || std::floating_point<T>;

template<size_t N, typename T>
class VectorT {
public:
    constexpr VectorT() noexcept { std::memset(el, 0, N); }

    // template<typename ...Args>
    template<class... T2, typename std::enable_if<sizeof...(T2) == N, int>::type = 0>
    constexpr explicit VectorT(const T2&... args) noexcept {
        static_assert(sizeof...(T2) == N, "VectorT: invalid number of arguments");

        const std::array<T, N>& arr = {args...};
        for (u32 i = 0; i < sizeof...(T2); i++) { el[i] = arr[i]; }
    }

    constexpr auto operator=(const VectorT& other) -> VectorT& {
        for (u32 i = 0; i < N; i++) { el[i] = other.el[i]; }
        return *this;
    }

    constexpr auto operator+(const VectorT& other) const -> VectorT { return this->add(other); }

    constexpr auto operator-(const VectorT& other) const -> VectorT { return this->sub(other); }

    constexpr auto operator*(const T& other) const -> VectorT { return this->mult(other); }

    constexpr auto operator/(const T& other) const -> VectorT { return this->div(other); }

    constexpr auto operator+=(const VectorT& other) -> VectorT& {
        *this = *this + other;
        return *this;
    }

    constexpr auto operator-=(const VectorT& other) -> VectorT& {
        *this = *this - other;
        return *this;
    }

    constexpr auto operator*=(const T& other) -> VectorT& {
        *this = *this * other;
        return *this;
    }

    constexpr auto operator/=(const T& other) -> VectorT& {
        *this = *this / other;
        return *this;
    }

    constexpr auto operator==(const VectorT& o) -> bool {
        bool result = true;
        for (u32 i = 0; i < N; i++) { result = result && (el[i] == o.el[i]); }
        return result;
        // return (x == other.x) && (y == other.y);
    }

    constexpr auto operator!=(const VectorT& o) -> bool { return !(*this == o); }

    constexpr auto add(const VectorT o) const -> VectorT {
        VectorT result;
        for (u32 i = 0; i < N; i++) { result.el[i] = el[i] + o.el[i]; }
        return result;
    }

    constexpr auto sub(const VectorT o) const -> VectorT {
        VectorT result;
        for (u32 i = 0; i < N; i++) { result.el[i] = el[i] - o.el[i]; }
        return result;
    }

    constexpr auto mult(const T o) const -> VectorT {
        VectorT result;
        for (u32 i = 0; i < N; i++) { result.el[i] = el[i] * o.el[i]; }
        return result;
    }

    constexpr auto div(const T o) const -> VectorT {
        VectorT result;
        for (u32 i = 0; i < N; i++) { result.el[i] = el[i] / o.el[i]; }
        return result;
    }

    constexpr auto dot(const VectorT& o) const -> T {
        T result;
        for (u32 i = 0; i < N; i++) { result += el[i] * o.el[i]; }
        return result;
    }

public:
    T el[N];
};

/*---------------------------
        Vec2
---------------------------*/

template<typename T>
class VectorT<2, T> {
public:
    constexpr VectorT() noexcept
        : x()
        , y() {}

    constexpr VectorT(const VectorT&) = default;
    constexpr VectorT(VectorT&&) = default;
    ~VectorT() = default;

    constexpr VectorT(const T x, const T y) noexcept
        : x(x)
        , y(y) {}

    constexpr auto operator=(const VectorT& other) -> VectorT& {
        this->x = other.x;
        this->y = other.y;
        return *this;
    }

    constexpr auto operator+(const VectorT& other) const -> VectorT { return this->add(other); }

    constexpr auto operator-(const VectorT& other) const -> VectorT { return this->sub(other); }

    constexpr auto operator*(const T& other) const -> VectorT { return this->mult(other); }

    constexpr auto operator/(const T& other) const -> VectorT { return this->div(other); }

    constexpr auto operator+=(const VectorT& other) -> VectorT& {
        *this = *this + other;
        return *this;
    }

    constexpr auto operator-=(const VectorT& other) -> VectorT& {
        *this = *this - other;
        return *this;
    }

    constexpr auto operator*=(const T& other) -> VectorT& {
        *this = *this * other;
        return *this;
    }

    constexpr auto operator/=(const T& other) -> VectorT& {
        *this = *this / other;
        return *this;
    }

    constexpr auto operator==(const VectorT& other) -> bool { return (x == other.x) && (y == other.y); }

    constexpr auto operator!=(const VectorT& other) -> bool { return !(*this == other); }

    constexpr auto add(const VectorT o) const -> VectorT { return VectorT(x + o.x, y + o.y); }

    constexpr auto sub(const VectorT o) const -> VectorT { return VectorT(x - o.x, y - o.y); }

    constexpr auto mult(const T o) const -> VectorT { return VectorT(x * o, y * o); }

    constexpr auto div(const T o) const -> VectorT { return VectorT(x / o, y / o); }

    constexpr auto dot(const VectorT& o) const -> T { return x * o.x + y * o.y; }

    // constexpr auto get_length() const {
    //	const T dot = this->dot(*this);
    //	return std::sqrt(dot);
    // }
    // constexpr auto get_normal() const -> VectorT {
    //	const f32& length = this->get_length();
    //	return VectorT(x / length, y / length, z / length);
    // }

public:
    union {
        T el[2];

        struct {
            T x, y;
        };

        struct {
            T w, h;
        };

        struct {
            T width, height;
        };
    };
};

/*---------------------------
        Vec3
---------------------------*/

template<typename T>
class VectorT<3, T> {
public:
    constexpr VectorT() noexcept
        : x()
        , y()
        , z() {}

    constexpr VectorT(const VectorT&) = default;
    constexpr VectorT(VectorT&&) = default;
    ~VectorT() = default;

    constexpr VectorT(const T x, const T y, const T z) noexcept
        : x(x)
        , y(y)
        , z(z) {}

    constexpr auto operator=(const VectorT& other) -> VectorT& {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
        return *this;
    }

    constexpr auto operator+(const VectorT& other) const -> VectorT { return this->add(other); }

    constexpr auto operator-(const VectorT& other) const -> VectorT { return this->sub(other); }

    constexpr auto operator*(const T& other) const -> VectorT { return this->mult(other); }

    constexpr auto operator/(const T& other) const -> VectorT { return this->div(other); }

    constexpr auto operator+=(const VectorT& other) -> VectorT& {
        *this = *this + other;
        return *this;
    }

    constexpr auto operator-=(const VectorT& other) -> VectorT& {
        *this = *this - other;
        return *this;
    }

    constexpr auto operator*=(const T& other) -> VectorT& {
        *this = *this * other;
        return *this;
    }

    constexpr auto operator/=(const T& other) -> VectorT& {
        *this = *this / other;
        return *this;
    }

    constexpr auto operator==(const VectorT& other) -> bool {
        return ((x == other.x) && (y == other.y) && (z == other.z));
    }

    constexpr auto operator!=(const VectorT& other) -> bool { return !(*this == other); }

    constexpr auto add(const VectorT o) const -> VectorT { return VectorT(x + o.x, y + o.y, z + o.z); }

    constexpr auto sub(const VectorT o) const -> VectorT { return VectorT(x - o.x, y - o.y, z - o.z); }

    constexpr auto mult(const T o) const -> VectorT { return VectorT(x * o, y * o, z * o); }

    constexpr auto div(const T o) const -> VectorT { return VectorT(x / o, y / o, z / o); }

    constexpr auto dot(const VectorT& o) const -> T { return x * o.x + y * o.y + z * o.z; }

    constexpr auto cross(const VectorT& o) const -> VectorT {
        return VectorT(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
    }

    constexpr auto get_length() const {
        const T dot = this->dot(*this);
        return dot * dot /*std::sqrt(dot)*/;
    }

    constexpr auto get_normal() const -> VectorT {
        const f32& length = static_cast<f32>(this->get_length());
        return VectorT(x / (T)length, y / (T)length, z / (T)length);
    }

public:
    union {
        T el[3];

        struct {
            T x, y, z;
        };

        struct {
            T r, g, b;
        };
    };
};

/*---------------------------
        Vec4
---------------------------*/

template<typename T>
class VectorT<4, T> {
public:
    constexpr VectorT() noexcept
        : x()
        , y()
        , z()
        , w() {}

    constexpr VectorT(const VectorT&) = default;
    constexpr VectorT(VectorT&&) = default;
    ~VectorT() = default;

    constexpr VectorT(const T x, const T y, const T z, const T w) noexcept
        : x(x)
        , y(y)
        , z(z)
        , w(w) {}

    constexpr auto operator=(const VectorT& other) -> VectorT& {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
        this->w = other.w;
        return *this;
    }

    constexpr auto operator+(const VectorT& other) const -> VectorT { return this->add(other); }

    constexpr auto operator-(const VectorT& other) const -> VectorT { return this->sub(other); }

    constexpr auto operator*(const T& other) const -> VectorT { return this->mult(other); }

    constexpr auto operator/(const T& other) const -> VectorT { return this->div(other); }

    constexpr auto operator+=(const VectorT& other) -> VectorT& {
        *this = *this + other;
        return *this;
    }

    constexpr auto operator-=(const VectorT& other) -> VectorT& {
        *this = *this - other;
        return *this;
    }

    constexpr auto operator*=(const T& other) -> VectorT& {
        *this = *this * other;
        return *this;
    }

    constexpr auto operator/=(const T& other) -> VectorT& {
        *this = *this / other;
        return *this;
    }

    constexpr auto operator==(const VectorT& other) -> bool {
        return ((x == other.x) && (y == other.y) && (z == other.z) && (w == other.w));
    }

    constexpr auto operator!=(const VectorT& other) -> bool { return !(*this == other); }

    constexpr auto add(const VectorT o) const -> VectorT { return VectorT(x + o.x, y + o.y, z + o.z, w + o.w); }

    constexpr auto sub(const VectorT o) const -> VectorT { return VectorT(x - o.x, y - o.y, z - o.z, w - o.w); }

    constexpr auto mult(const T o) const -> VectorT { return VectorT(x * o, y * o, z * o, w * o); }

    constexpr auto div(const T o) const -> VectorT { return VectorT(x / o, y / o, z / o, w / o); }

    constexpr auto dot(const VectorT& o) const -> T { return x * o.x + y * o.y + z * o.z + w * o.w; }

    // constexpr auto get_length() const {
    //	const T dot = this->dot(*this);
    //	return std::sqrt(dot);
    // }
    // constexpr auto get_normal() const -> Vec3 {
    //	const f32& length = this->get_length();
    //	return Vec3(x / length, y / length, z / length);
    // }

public:
    union {
        T el[4];

        struct {
            T x, y, z, w;
        };

        struct {
            T r, g, b, a;
        };
    };
};

/*---------------------------

---------------------------*/

using v2f32 = VectorT<2, f32>;
using v3f32 = VectorT<3, f32>;
using v4f32 = VectorT<4, f32>;

using v2f64 = VectorT<2, f64>;
using v3f64 = VectorT<3, f64>;
using v4f64 = VectorT<4, f64>;

using v2u8 = VectorT<2, u8>;
using v3u8 = VectorT<3, u8>;
using v4u8 = VectorT<4, u8>;

using v2u16 = VectorT<2, u16>;
using v3u16 = VectorT<3, u16>;
using v4u16 = VectorT<4, u16>;

using v2u32 = VectorT<2, u32>;
using v3u32 = VectorT<3, u32>;
using v4u32 = VectorT<4, u32>;

using v2u64 = VectorT<2, u64>;
using v3u64 = VectorT<3, u64>;
using v4u64 = VectorT<4, u64>;

using v2i8 = VectorT<2, i8>;
using v3i8 = VectorT<3, i8>;
using v4i8 = VectorT<4, i8>;

using v2i16 = VectorT<2, i16>;
using v3i16 = VectorT<3, i16>;
using v4i16 = VectorT<4, i16>;

using v2i32 = VectorT<2, i32>;
using v3i32 = VectorT<3, i32>;
using v4i32 = VectorT<4, i32>;

using v2i64 = VectorT<2, i64>;
using v3i64 = VectorT<3, i64>;
using v4i64 = VectorT<4, i64>;

using v2i = v2i32;
using v3i = v3i32;
using v4i = v4i32;

using v2u = v2u32;
using v3u = v3u32;
using v4u = v4u32;

using v2 = v2f32;
using v3 = v3f32;
using v4 = v4f32;
JF_PRAGMA_POP
} // namespace JadeFrame