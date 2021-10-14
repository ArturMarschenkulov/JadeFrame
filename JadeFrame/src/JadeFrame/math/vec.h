#pragma once
#include "JadeFrame/defines.h"
#include <concepts>
#include <type_traits>
#include <cmath>

namespace JadeFrame {


template <class T>
concept scalar = std::is_scalar_v<T>;

template <class T>
concept number = std::integral<T> || std::floating_point<T>;



template<size_t N, typename T>
requires number<T>
class Vec_Base {
public:
public:
};


/*---------------------------
	Vec2
---------------------------*/

template<typename T>
class Vec_Base<2, T> {
public:
	constexpr Vec_Base() noexcept : x(0.0f), y(0.0f) {

	}
	constexpr Vec_Base(const T x, const T y) noexcept : x(x), y(y) {

	}

	constexpr auto operator=(const Vec_Base& other) -> Vec_Base& {
		this->x = other.x;
		this->y = other.y;
		return *this;
	}

	constexpr auto operator+(const Vec_Base& other) const -> Vec_Base {
		return this->add(other);
	}
	constexpr auto operator-(const Vec_Base& other) const -> Vec_Base {
		return this->sub(other);
	}
	constexpr auto operator*(const T& other) const -> Vec_Base {
		return this->mult(other);
	}
	constexpr auto operator/(const T& other) const -> Vec_Base {
		return this->div(other);
	}

	constexpr auto operator+=(const Vec_Base& other) -> Vec_Base& {
		*this = *this + other;
		return *this;
	}
	constexpr auto operator-=(const Vec_Base& other) -> Vec_Base& {
		*this = *this - other;
		return *this;
	}
	constexpr auto operator*=(const T& other) -> Vec_Base& {
		*this = *this * other;
		return *this;
	}
	constexpr auto operator/=(const T& other) -> Vec_Base& {
		*this = *this / other;
		return *this;
	}
	constexpr auto operator==(const Vec_Base& other) -> bool {
		return (x == other.x) && (y == other.y);
	}
	constexpr auto operator!=(const Vec_Base& other) -> bool {
		return !(*this == other);
	}

	constexpr auto add(const Vec_Base o) const -> Vec_Base {
		return Vec_Base(x + o.x, y + o.y);
	}
	constexpr auto sub(const Vec_Base o) const -> Vec_Base {
		return Vec_Base(x - o.x, y - o.y);
	}
	constexpr auto mult(const T o) const -> Vec_Base {
		return Vec_Base(x * o, y * o);
	}
	constexpr auto div(const T o) const -> Vec_Base {
		return Vec_Base(x / o, y / o);
	}
	constexpr auto dot(const Vec_Base& o) const -> T {
		return x * o.x + y * o.y;
	}
	//constexpr auto get_length() const {
	//	const T dot = this->dot(*this);
	//	return std::sqrt(dot);
	//}
	//constexpr auto get_normal() const -> Vec_Base {
	//	const f32& length = this->get_length();
	//	return Vec_Base(x / length, y / length, z / length);
	//}
public:
	union {
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
class Vec_Base<3, T> {
public:
	constexpr Vec_Base() noexcept : x(0.0f), y(0.0f), z(0.0f) {

	}
	constexpr Vec_Base(const T x, const T y, const T z) noexcept : x(x), y(y), z(z) {

	}

	constexpr auto operator=(const Vec_Base& other) -> Vec_Base& {
		this->x = other.x;
		this->y = other.y;
		this->z = other.z;
		return *this;
	}

	constexpr auto operator+(const Vec_Base& other) const -> Vec_Base {
		return this->add(other);
	}
	constexpr auto operator-(const Vec_Base& other) const -> Vec_Base {
		return this->sub(other);
	}
	constexpr auto operator*(const T& other) const -> Vec_Base {
		return this->mult(other);
	}
	constexpr auto operator/(const T& other) const -> Vec_Base {
		return this->div(other);
	}

	constexpr auto operator+=(const Vec_Base& other) -> Vec_Base& {
		*this = *this + other;
		return *this;
	}
	constexpr auto operator-=(const Vec_Base& other) -> Vec_Base& {
		*this = *this - other;
		return *this;
	}
	constexpr auto operator*=(const T& other) -> Vec_Base& {
		*this = *this * other;
		return *this;
	}
	constexpr auto operator/=(const T& other) -> Vec_Base& {
		*this = *this / other;
		return *this;
	}
	constexpr auto operator==(const Vec_Base& other) -> bool {
		return ((x == other.x) && (y == other.y) && (z == other.z));
	}
	constexpr auto operator!=(const Vec_Base& other) -> bool {
		return !(*this == other);
	}

	constexpr auto add(const Vec_Base o) const -> Vec_Base {
		return Vec_Base(x + o.x, y + o.y, z + o.z);
	}
	constexpr auto sub(const Vec_Base o) const -> Vec_Base {
		return Vec_Base(x - o.x, y - o.y, z - o.z);
	}
	constexpr auto mult(const T o) const -> Vec_Base {
		return Vec_Base(x * o, y * o, z * o);
	}
	constexpr auto div(const T o) const -> Vec_Base {
		return Vec_Base(x / o, y / o, z / o);
	}
	constexpr auto dot(const Vec_Base& o) const -> T {
		return x * o.x + y * o.y + z * o.z;
	}
	constexpr auto cross(const Vec_Base& o) const -> Vec_Base {
		return Vec_Base(
			y * o.z - z * o.y,
			z * o.x - x * o.z,
			x * o.y - y * o.x
		);
	}
	constexpr auto get_length() const {
		const T dot = this->dot(*this);
		return dot * dot/*std::sqrt(dot)*/;
	}
	constexpr auto get_normal() const -> Vec_Base {
		const f32& length = this->get_length();
		return Vec_Base(x / length, y / length, z / length);
	}
public:
	union {
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
class Vec_Base<4, T> {
public:
	constexpr Vec_Base() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {

	}
	constexpr Vec_Base(const T x, const T y, const T z, const T w) noexcept : x(x), y(y), z(z), w(w) {

	}

	constexpr auto operator=(const Vec_Base& other) -> Vec_Base& {
		this->x = other.x;
		this->y = other.y;
		this->z = other.z;
		this->w = other.w;
		return *this;
	}

	constexpr auto operator+(const Vec_Base& other) const -> Vec_Base {
		return this->add(other);
	}
	constexpr auto operator-(const Vec_Base& other) const -> Vec_Base {
		return this->sub(other);
	}
	constexpr auto operator*(const T& other) const -> Vec_Base {
		return this->mult(other);
	}
	constexpr auto operator/(const T& other) const -> Vec_Base {
		return this->div(other);
	}

	constexpr auto operator+=(const Vec_Base& other) -> Vec_Base& {
		*this = *this + other;
		return *this;
	}
	constexpr auto operator-=(const Vec_Base& other) -> Vec_Base& {
		*this = *this - other;
		return *this;
	}
	constexpr auto operator*=(const T& other) -> Vec_Base& {
		*this = *this * other;
		return *this;
	}
	constexpr auto operator/=(const T& other) -> Vec_Base& {
		*this = *this / other;
		return *this;
	}
	constexpr auto operator==(const Vec_Base& other) -> bool {
		return ((x == other.x) && (y == other.y) && (z == other.z) && (w == other.w));
	}
	constexpr auto operator!=(const Vec_Base& other) -> bool {
		return !(*this == other);
	}

	constexpr auto add(const Vec_Base o) const -> Vec_Base {
		return Vec_Base(x + o.x, y + o.y, z + o.z, w + o.w);
	}
	constexpr auto sub(const Vec_Base o) const -> Vec_Base {
		return Vec_Base(x - o.x, y - o.y, z - o.z, w - o.w);
	}
	constexpr auto mult(const T o) const -> Vec_Base {
		return Vec_Base(x * o, y * o, z * o, w * o);
	}
	constexpr auto div(const T o) const -> Vec_Base {
		return Vec_Base(x / o, y / o, z / o, w / o);
	}
	constexpr auto dot(const Vec_Base& o) const -> T {
		return x * o.x + y * o.y + z * o.z + w * o.w;
	}
	//constexpr auto get_length() const {
	//	const T dot = this->dot(*this);
	//	return std::sqrt(dot);
	//}
	//constexpr auto get_normal() const -> Vec3 {
	//	const f32& length = this->get_length();
	//	return Vec3(x / length, y / length, z / length);
	//}
public:
	union {
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

using v2f32 = Vec_Base<2, f32>;
using v3f32 = Vec_Base<3, f32>;
using v4f32 = Vec_Base<4, f32>;

using v2f64 = Vec_Base<2, f64>;
using v3f64 = Vec_Base<3, f64>;
using v4f64 = Vec_Base<4, f64>;

using v2u8 = Vec_Base<2, u8>;
using v3u8 = Vec_Base<3, u8>;
using v4u8 = Vec_Base<4, u8>;

using v2u16 = Vec_Base<2, u16>;
using v3u16 = Vec_Base<3, u16>;
using v4u16 = Vec_Base<4, u16>;

using v2u32 = Vec_Base<2, u32>;
using v3u32 = Vec_Base<3, u32>;
using v4u32 = Vec_Base<4, u32>;

using v2u64 = Vec_Base<2, u64>;
using v3u64 = Vec_Base<3, u64>;
using v4u64 = Vec_Base<4, u64>;

using v2i8 = Vec_Base<2, i8>;
using v3i8 = Vec_Base<3, i8>;
using v4i8 = Vec_Base<4, i8>;

using v2i16 = Vec_Base<2, i16>;
using v3i16 = Vec_Base<3, i16>;
using v4i16 = Vec_Base<4, i16>;

using v2i32 = Vec_Base<2, i32>;
using v3i32 = Vec_Base<3, i32>;
using v4i32 = Vec_Base<4, i32>;

using v2i64 = Vec_Base<2, i64>;
using v3i64 = Vec_Base<3, i64>;
using v4i64 = Vec_Base<4, i64>;

using v2 = v2f32;
using v3 = v3f32;
using v4 = v4f32;

}