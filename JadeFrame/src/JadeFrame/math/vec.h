#pragma once
#include "JadeFrame/prelude.h"
#include "JadeFrame/types.h"
// #include <concepts>
// #include <cmath>
#include <cstring>
#include <type_traits>

namespace JadeFrame {
JF_PRAGMA_PUSH

// To maintain the high convenience of anonymous structs and unions, we disable the
// warnings
#if defined(__clang__)
    #pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
    #pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(__GNUC__)
    #pragma GCC diagnostic ignored "-Wpedantic"
    #pragma GCC diagnostic ignored "-Wshadow"
#elif defined(_MSC_VER)
    #pragma warning(disable : 4201)
#endif

// template <typename T>
// concept scalar = std::is_scalar_v<T>;
//
// template <typename T>
// concept number = std::integral<T> || std::floating_point<T>;

template<typename T>
    requires std::integral<T> || std::floating_point<T>
class _v2 {
public:
    constexpr _v2() noexcept = default;
    constexpr _v2(const _v2&) = default;
    constexpr auto operator=(const _v2& other) -> _v2& = default;
    constexpr _v2(_v2&&) = default;
    constexpr auto operator=(_v2&& other) noexcept -> _v2& = default;
    constexpr ~_v2() = default;

public: // operators
    constexpr auto operator+(const _v2& other) const -> _v2 {
        return _v2(x + other.x, y + other.y);
    }

    constexpr auto operator-(const _v2& other) const -> _v2 {
        return _v2(x - other.x, y - other.y);
    }

    // this is a dot product
    constexpr auto operator*(const T& other) const -> _v2 {
        return _v2(x * other, y * other);
    }

    constexpr auto operator/(const T& other) const -> _v2 {
        return _v2(x / other, y / other);
    }

    constexpr auto operator+=(const _v2& other) -> _v2& {
        *this = *this + other;
        return *this;
    }

    constexpr auto operator-=(const _v2& other) -> _v2& {
        *this = *this - other;
        return *this;
    }

    constexpr auto operator*=(const T& other) -> _v2& {
        *this = *this * other;
        return *this;
    }

    constexpr auto operator/=(const T& other) -> _v2& {
        *this = *this / other;
        return *this;
    }

    constexpr auto operator==(const _v2& other) -> bool {
        return (x == other.x) && (y == other.y);
    }

    constexpr auto operator!=(const _v2& other) -> bool { return !(*this == other); }

    constexpr auto operator-() const -> _v2 { return _v2(-x, -y); }

    // constexpr auto operator[](const u32 index) const -> T& {
    //     if (index == 0) { return x; }
    //     if (index == 1) { return y; }
    //     assert(false && "index out of bounds");
    // }

    // constexpr auto operator[](const u32 index) -> T& {
    //     if (index == 0) { return x; }
    //     if (index == 1) { return y; }
    //     assert(false && "index out of bounds");
    // }

public: // named operations
    constexpr auto add(const _v2 o) const -> _v2 { return _v2(x + o.x, y + o.y); }

    constexpr auto sub(const _v2 o) const -> _v2 { return _v2(x - o.x, y - o.y); }

    constexpr auto mult(const T o) const -> _v2 { return _v2(x * o, y * o); }

    constexpr auto div(const T o) const -> _v2 { return _v2(x / o, y / o); }

    constexpr auto dot(const _v2& o) const -> T { return x * o.x + y * o.y; }

public:
    T x = {}; // width, r
    T y = {}; // height, g

private:
    constexpr _v2(const T x, const T y) noexcept
        : x(x)
        , y(y) {}

public:
    constexpr static auto splat(const T& value) noexcept -> _v2 {
        return _v2::create(value, value);
    }

    constexpr static auto create(const T x, const T y) noexcept -> _v2 {
        return _v2(x, y);
    }

    constexpr static auto zero() noexcept -> _v2 { return _v2(0, 0); }

    constexpr static auto one() noexcept -> _v2 { return _v2(1, 1); }

    constexpr static auto X() noexcept -> _v2 { return _v2(1, 0); }

    constexpr static auto Y() noexcept -> _v2 { return _v2(0, 1); }

    constexpr static auto NEG_X() noexcept -> _v2 { return -_v2::X(); }

    constexpr static auto NEG_Y() noexcept -> _v2 { return -_v2::Y(); }
};

template<typename T>
class _v3 {
public:
    constexpr _v3() noexcept = default;
    constexpr _v3(const _v3&) = default;
    constexpr auto operator=(const _v3& other) -> _v3& = default;
    constexpr _v3(_v3&&) = default;
    constexpr auto operator=(_v3&& other) noexcept -> _v3& = default;
    constexpr ~_v3() = default;

public: // operators
    constexpr auto operator+(const _v3& other) const -> _v3 {
        return _v3::create(x + other.x, y + other.y, z + other.z);
    }

    constexpr auto operator-(const _v3& other) const -> _v3 {
        return _v3::create(x - other.x, y - other.y, z - other.z);
    }

    constexpr auto operator*(const T& other) const -> _v3 {
        return _v3::create(x * other, y * other, z * other);
    }

    constexpr auto operator/(const T& other) const -> _v3 {
        return _v3::create(x / other, y / other, z / other);
    }

    constexpr auto operator*(const _v3& other) const -> _v3 {
        return _v3::create(x * other.x, y * other.y, z * other.z);
    }

    constexpr auto operator+=(const _v3& other) -> _v3& {
        *this = *this + other;
        return *this;
    }

    constexpr auto operator-=(const _v3& other) -> _v3& {
        *this = *this - other;
        return *this;
    }

    constexpr auto operator*=(const T& other) -> _v3& {
        *this = *this * other;
        return *this;
    }

    constexpr auto operator/=(const T& other) -> _v3& {
        *this = *this / other;
        return *this;
    }

    constexpr auto operator==(const _v3& other) const -> bool {
        const f64 tolerance = 1e-6;
        return (std::fabs(x - other.x) < tolerance) &&
               (std::fabs(y - other.y) < tolerance) &&
               (std::fabs(z - other.z) < tolerance);
    }

    constexpr auto operator!=(const _v3& other) const -> bool {
        return !(*this == other);
    }

    constexpr auto operator-() const -> _v3 { return _v3::create(-x, -y, -z); }

    // constexpr auto operator[](const u32 index) const -> T& {
    //     if (index == 0) { return x; }
    //     if (index == 1) { return y; }
    //     if (index == 2) { return z; }
    //     assert(false && "index out of bounds");
    // }

    // constexpr auto operator[](const u32 index) -> T& {
    //     if (index == 0) { return x; }
    //     if (index == 1) { return y; }
    //     if (index == 2) { return z; }
    //     assert(false && "index out of bounds");
    // }

public: // named operations
    constexpr auto add(const _v3 o) const -> _v3 {
        return _v3(x + o.x, y + o.y, z + o.z);
    }

    constexpr auto sub(const _v3 o) const -> _v3 {
        return _v3(x - o.x, y - o.y, z - o.z);
    }

    constexpr auto mult(const T o) const -> _v3 { return _v3(x * o, y * o, z * o); }

    constexpr auto div(const T o) const -> _v3 { return _v3(x / o, y / o, z / o); }

    constexpr auto dot(const _v3& o) const -> T { return x * o.x + y * o.y + z * o.z; }

    constexpr auto cross(const _v3& o) const -> _v3 {
        return _v3::create(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
    }

    constexpr auto length() const -> T {
        const T dot = this->dot(*this);
        return std::sqrt(dot);
    }

    constexpr auto normalize() const -> _v3 {
        const T   length = static_cast<T>(this->length());
        const T   inv_length = 1 / length;
        const _v3 normalized = *this * inv_length;
        return normalized;
    }

    constexpr auto reciprocal() const -> _v3 { return _v3::create(1 / x, 1 / y, 1 / z); }

public:
    T x = {}; // width, r
    T y = {}; // height, g
    T z = {}; // depth, b

    // private:
    // constexpr _v3(const T x, const T y, const T z) noexcept
    //     : x(x)
    //     , y(y)
    //     , z(z) {}
    constexpr _v3(const T x, const T y, const T z) noexcept = delete;

public:
    constexpr static auto create(const T x, const T y, const T z) noexcept -> _v3 {
        _v3 result;
        result.x = x;
        result.y = y;
        result.z = z;
        return result;
    }

    constexpr static auto splat(const T& value) -> _v3 {
        return _v3::create(value, value, value);
    }

    constexpr static auto zero() -> _v3 { return _v3::create(0, 0, 0); }

    constexpr static auto one() -> _v3 { return _v3::create(1, 1, 1); }

    constexpr static auto X() -> _v3 { return _v3::create(1, 0, 0); }

    constexpr static auto Y() -> _v3 { return _v3::create(0, 1, 0); }

    constexpr static auto Z() -> _v3 { return _v3::create(0, 0, 1); }

    constexpr static auto NEG_X() -> _v3 { return -_v3::X(); }

    constexpr static auto NEG_Y() -> _v3 { return -_v3::Y(); }

    constexpr static auto NEG_Z() -> _v3 { return -_v3::Z(); }
};

template<typename T>
class _v4 {
public:
    T x;
    T y;
    T z;
    T w;

public:
    constexpr static auto
    create(const T x, const T y, const T z, const T w) noexcept -> _v4 {
        return _v4(x, y, z, w);
    }

    constexpr static auto from_v3(const _v3<T>& xyz, const T w) noexcept -> _v4 {
        return _v4(xyz.x, xyz.y, xyz.z, w);
    }

    constexpr static auto splat(const T& value) -> _v4 {
        return _v4(value, value, value, value);
    }

    constexpr static auto zero() -> _v4 { return _v4::splat(0); }

    constexpr static auto one() -> _v4 { return _v4::splat(1); }

    constexpr static auto X() -> _v4 { return _v4(1, 0, 0, 0); }

    constexpr static auto Y() -> _v4 { return _v4(0, 1, 0, 0); }

    constexpr static auto Z() -> _v4 { return _v4(0, 0, 1, 0); }

    constexpr static auto W() -> _v4 { return _v4(0, 0, 0, 1); }

    constexpr static auto NEG_X() -> _v4 { return -_v4::X(); }

    constexpr static auto NEG_Y() -> _v4 { return -_v4::Y(); }

    constexpr static auto NEG_Z() -> _v4 { return -_v4::Z(); }

    constexpr static auto NEG_W() -> _v4 { return -_v4::W(); }

    constexpr auto length() const -> T {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    constexpr auto xyz() const -> _v3<T> { return _v3<T>::create(x, y, z); }

public:
    constexpr auto operator-() const -> _v4 { return _v4(-x, -y, -z, -w); }

    constexpr auto operator+(const _v4& other) const -> _v4 {
        return _v4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    constexpr auto operator-(const _v4& other) const -> _v4 {
        return _v4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    constexpr auto operator*(const _v4& other) const -> _v4 {
        return _v4(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    constexpr auto operator/(const _v4& other) const -> _v4 {
        return _v4(x / other.x, y / other.y, z / other.z, w / other.w);
    }

    constexpr auto operator*(const T& other) const -> _v4 {
        return _v4(x * other, y * other, z * other, w * other);
    }

    constexpr auto operator/(const T& other) const -> _v4 {
        return _v4(x / other, y / other, z / other, w / other);
    }

    constexpr auto operator+=(const _v4& other) -> _v4& {
        *this = *this + other;
        return *this;
    }

    constexpr auto operator-=(const _v4& other) -> _v4& {
        *this = *this - other;
        return *this;
    }

    constexpr auto operator*=(const T& other) -> _v4& {
        *this = *this * other;
        return *this;
    }

    constexpr auto operator/=(const T& other) -> _v4& {
        *this = *this / other;
        return *this;
    }

    constexpr auto operator==(const _v4& other) const -> bool {
        const f64 tolerance = 1e-6;
        return std::fabs(x - other.x) < tolerance && std::fabs(y - other.y) < tolerance &&
               std::fabs(z - other.z) < tolerance && std::fabs(w - other.w) < tolerance;
    }

    constexpr auto operator!=(const _v4& other) const -> bool {
        return !(*this == other);
    }

    constexpr auto reciprocal() const -> _v4 { return _v4(1 / x, 1 / y, 1 / z, 1 / w); }
};

template<typename T>
auto operator<<(std::ostream& os, const _v3<T>& v) -> std::ostream& {
    os << '{' << v.x << ", " << v.y << ", " << v.z << '}';
    return os;
}

template<typename T>
auto operator<<(std::ostream& os, const _v4<T>& v) -> std::ostream& {
    os << '{' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << '}';
    return os;
}

template<size_t N, typename T>
class vector_t {
public:
    constexpr vector_t() noexcept { std::memset(el, 0, N); }

    constexpr vector_t(const vector_t& other) noexcept {
        for (u32 i = 0; i < N; i++) { el[i] = other.el[i]; }
    }

    auto operator=(const vector_t& other) -> vector_t& {
        if (this == &other) { return *this; }
        for (u32 i = 0; i < N; i++) { el[i] = other.el[i]; }
        return *this;
    }

    // accessors

public:
    template<typename T2>
        requires(N > 0)
    auto x() const -> T& {
        return el[0];
    }

    template<typename T2>
        requires(N > 1)
    auto y() const -> T& {
        return el[1];
    }

    template<typename T2>
        requires(N > 2)
    auto z() const -> T& {
        return el[2];
    }

    template<typename T2>
        requires(N > 3)
    auto w() const -> T& {
        return el[3];
    }

public:
    T el[N];
};

template<size_t N, typename T>
class VectorT {
public:
    constexpr VectorT() noexcept { std::memset(el, 0, N); }

    constexpr auto operator=(const VectorT& other) -> VectorT& {
        if (this == &other) { return *this; }
        for (u32 i = 0; i < N; i++) { el[i] = other.el[i]; }
        return *this;
    }

    // template<typename ...Args>
    template<class... T2, std::enable_if_t<sizeof...(T2) == N, int> = 0>
    constexpr explicit VectorT(const T2&... args) noexcept {
        static_assert(sizeof...(T2) == N, "VectorT: invalid number of arguments");

        const std::array<T, N>& arr = {args...};
        for (u32 i = 0; i < sizeof...(T2); i++) { el[i] = arr[i]; }
    }

    constexpr auto operator+(const VectorT& other) const -> VectorT {
        return this->add(other);
    }

    constexpr auto operator-(const VectorT& other) const -> VectorT {
        return this->sub(other);
    }

    constexpr auto operator*(const T& other) const -> VectorT {
        return this->mult(other);
    }

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
    constexpr VectorT() noexcept = default;
    constexpr VectorT(const VectorT&) = default;
    constexpr auto operator=(const VectorT& other) -> VectorT& = default;
    constexpr VectorT(VectorT&&) = default;
    constexpr auto operator=(VectorT&& other) noexcept -> VectorT& = default;
    ~VectorT() = default;

    constexpr VectorT(const T x, const T y) noexcept
        : x(x)
        , y(y) {}

    constexpr auto operator+(const VectorT& other) const -> VectorT {
        return this->add(other);
    }

    constexpr auto operator-(const VectorT& other) const -> VectorT {
        return this->sub(other);
    }

    constexpr auto operator*(const T& other) const -> VectorT {
        return this->mult(other);
    }

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
        return (x == other.x) && (y == other.y);
    }

    constexpr auto operator!=(const VectorT& other) -> bool { return !(*this == other); }

    constexpr auto add(const VectorT o) const -> VectorT {
        return VectorT(x + o.x, y + o.y);
    }

    constexpr auto sub(const VectorT o) const -> VectorT {
        return VectorT(x - o.x, y - o.y);
    }

    constexpr auto mult(const T o) const -> VectorT { return VectorT(x * o, y * o); }

    constexpr auto div(const T o) const -> VectorT { return VectorT(x / o, y / o); }

    constexpr auto dot(const VectorT& o) const -> T { return x * o.x + y * o.y; }

    // constexpr auto get_length() const {
    //	const T dot = this->dot(*this);
    //	return std::sqrt(dot);
    // }
    // constexpr auto normalize() const -> VectorT {
    //	const f32& length = this->get_length();
    //	return VectorT(x / length, y / length, z / length);
    // }

    // create a template concept thingy where if the T is unsigned, it will not call
    // `std::abs`, otherwise it will.

    constexpr auto abs() const -> VectorT { return VectorT(std::abs(x), std::abs(y)); }

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
    constexpr VectorT() noexcept = default;
    constexpr VectorT(const VectorT&) = default;
    constexpr auto operator=(const VectorT& other) -> VectorT& = default;
    constexpr VectorT(VectorT&&) = default;
    constexpr auto operator=(VectorT&& other) noexcept -> VectorT& = default;
    ~VectorT() = default;

    constexpr VectorT(const T x, const T y, const T z) noexcept
        : x(x)
        , y(y)
        , z(z) {}

    constexpr auto operator+(const VectorT& other) const -> VectorT {
        return this->add(other);
    }

    constexpr auto operator-(const VectorT& other) const -> VectorT {
        return this->sub(other);
    }

    constexpr auto operator*(const T& other) const -> VectorT {
        return this->mult(other);
    }

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

    constexpr auto add(const VectorT o) const -> VectorT {
        return VectorT(x + o.x, y + o.y, z + o.z);
    }

    constexpr auto sub(const VectorT o) const -> VectorT {
        return VectorT(x - o.x, y - o.y, z - o.z);
    }

    constexpr auto mult(const T o) const -> VectorT {
        return VectorT(x * o, y * o, z * o);
    }

    constexpr auto div(const T o) const -> VectorT {
        return VectorT(x / o, y / o, z / o);
    }

    constexpr auto dot(const VectorT& o) const -> T {
        return x * o.x + y * o.y + z * o.z;
    }

    constexpr auto cross(const VectorT& o) const -> VectorT {
        return VectorT(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
    }

    constexpr auto get_length() const {
        const T dot = this->dot(*this);
        return dot * dot /*std::sqrt(dot)*/;
    }

    constexpr auto normalize() const -> VectorT {
        const T length = static_cast<T>(this->get_length());
        return VectorT(x / length, y / length, z / length);
    }

public:
    constexpr static auto splat(const T& value) -> VectorT {
        return VectorT(value, value, value);
    }

    constexpr static auto zero() -> VectorT { return VectorT(0, 0, 0); }

    constexpr static auto one() -> VectorT { return VectorT(1, 1, 1); }

    constexpr static auto X() -> VectorT { return VectorT(1, 0, 0); }

    constexpr static auto Y() -> VectorT { return VectorT(0, 1, 0); }

    constexpr static auto Z() -> VectorT { return VectorT(0, 0, 1); }

    constexpr static auto NEG_X() -> VectorT { return VectorT(-1, 0, 0); }

    constexpr static auto NEG_Y() -> VectorT { return VectorT(0, -1, 0); }

    constexpr static auto NEG_Z() -> VectorT { return VectorT(0, 0, -1); }

    // constexpr static VectorT ZERO = VectorT(0, 0, 0);
    // constexpr static VectorT ONE = VectorT(1, 1, 1);
    // constexpr static VectorT X = VectorT(1, 0, 0);
    // constexpr static VectorT Y = VectorT(0, 1, 0);
    // constexpr static VectorT Z = VectorT(0, 0, 1);
    // constexpr static VectorT NEG_X = VectorT(-1, 0, 0);
    // constexpr static VectorT NEG_Y = VectorT(0, -1, 0);
    // constexpr static VectorT NEG_Z = VectorT(0, 0, -1);

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
    constexpr auto operator=(const VectorT& other) -> VectorT& = default;
    constexpr VectorT(VectorT&&) = default;
    constexpr auto operator=(VectorT&& other) noexcept -> VectorT& = default;
    ~VectorT() = default;

    constexpr VectorT(const T x_, const T y_, const T z_, const T w_) noexcept
        : x(x_)
        , y(y_)
        , z(z_)
        , w(w_) {}

    constexpr auto operator+(const VectorT& other) const -> VectorT {
        return this->add(other);
    }

    constexpr auto operator-(const VectorT& other) const -> VectorT {
        return this->sub(other);
    }

    constexpr auto operator*(const T& other) const -> VectorT {
        return this->mult(other);
    }

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

    constexpr auto add(const VectorT o) const -> VectorT {
        return VectorT(x + o.x, y + o.y, z + o.z, w + o.w);
    }

    constexpr auto sub(const VectorT o) const -> VectorT {
        return VectorT(x - o.x, y - o.y, z - o.z, w - o.w);
    }

    constexpr auto mult(const T o) const -> VectorT {
        return VectorT(x * o, y * o, z * o, w * o);
    }

    constexpr auto div(const T o) const -> VectorT {
        return VectorT(x / o, y / o, z / o, w / o);
    }

    constexpr auto dot(const VectorT& o) const -> T {
        return x * o.x + y * o.y + z * o.z + w * o.w;
    }

    // constexpr auto get_length() const {
    //	const T dot = this->dot(*this);
    //	return std::sqrt(dot);
    // }
    // constexpr auto normalize() const -> Vec3 {
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

using v2f32 = _v2<f32>;
using v3f32 = _v3<f32>;
using v4f32 = _v4<f32>;

using v2f64 = _v2<f64>;
using v3f64 = _v3<f64>;
using v4f64 = _v4<f64>;

using v2u8 = _v2<u8>;
using v3u8 = _v3<u8>;
using v4u8 = _v4<u8>;

using v2u16 = _v2<u16>;
using v3u16 = _v3<u16>;
using v4u16 = _v4<u16>;

using v2u32 = _v2<u32>;
using v3u32 = _v3<u32>;
using v4u32 = _v4<u32>;

using v2u64 = _v2<u64>;
using v3u64 = _v3<u64>;
using v4u64 = _v4<u64>;

using v2i8 = _v2<i8>;
using v3i8 = _v3<i8>;
using v4i8 = _v4<i8>;

using v2i16 = _v2<i16>;
using v3i16 = _v3<i16>;
using v4i16 = _v4<i16>;

using v2i32 = _v2<i32>;
using v3i32 = _v3<i32>;
using v4i32 = _v4<i32>;

using v2i64 = _v2<i64>;
using v3i64 = _v3<i64>;
using v4i64 = _v4<i64>;

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