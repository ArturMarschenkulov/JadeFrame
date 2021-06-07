#pragma once
#include "JadeFrame/defines.h"
#include "vec_2.h"
#include "vec_4.h"
#include <cmath>
class Vec3 {
public:
	constexpr Vec3() noexcept;
	constexpr Vec3(const f32 x, const f32 y, const f32 z) noexcept;
	constexpr Vec3(const Vec2& vec, const f32 z) noexcept;
	constexpr Vec3(const Vec3& vec) noexcept;
	constexpr explicit Vec3(const f32 scalar) noexcept;
	constexpr explicit Vec3(const Vec4& vec) noexcept;

	constexpr auto operator=(const Vec3& other)->Vec3&;
	constexpr auto operator+(const Vec3& other) const->Vec3;
	constexpr auto operator-(const Vec3& other) const->Vec3;
	constexpr auto operator*(const f32& other) const->Vec3;
	constexpr auto operator/(const f32& other) const->Vec3;

	constexpr auto operator+=(const Vec3& other)->Vec3&;
	constexpr auto operator-=(const Vec3& other)->Vec3&;
	constexpr auto operator*=(const f32& other)->Vec3&;
	constexpr auto operator/=(const f32& other)->Vec3&;

	constexpr auto operator==(const Vec3& other) -> bool;
	constexpr auto operator!=(const Vec3& other) -> bool;


	constexpr auto add(const Vec3 o) const->Vec3;
	constexpr auto sub(const Vec3 o) const->Vec3;
	constexpr auto mult(const f32 o) const->Vec3;
	constexpr auto div(const f32 o) const->Vec3;
	constexpr auto get_length() const->f32;
	constexpr auto dot(const Vec3& o) const -> f32;
	constexpr auto cross(const Vec3& o) const->Vec3;
	constexpr auto get_normal() const->Vec3;
public:
	f32 x;
	f32 y;
	f32 z;
};

// IMPLEMENTATION

inline constexpr Vec3::Vec3() noexcept : x(0.0f), y(0.0f), z(0.0f) {
}
inline constexpr Vec3::Vec3(const f32 scalar) noexcept : x(scalar), y(scalar), z(scalar) {
}
inline constexpr Vec3::Vec3(const f32 x, const f32 y, const f32 z) noexcept : x(x), y(y), z(z) {
}
inline constexpr Vec3::Vec3(const Vec2& vec2, f32 z) noexcept : x(vec2.x), y(vec2.y), z(z) {
}
inline constexpr Vec3::Vec3(const Vec4& vec4) noexcept : x(vec4.x), y(vec4.y), z(vec4.z) {
}
inline constexpr Vec3::Vec3(const Vec3& vec) noexcept : x(vec.x), y(vec.y), z(vec.z) {
}


inline constexpr auto Vec3::operator=(const Vec3& other) -> Vec3& {
	this->x = other.x;
	this->y = other.y;
	this->z = other.z;
	return *this;
}
inline constexpr auto Vec3::operator+(const Vec3& other) const -> Vec3 {
	return this->add(other);
}
inline constexpr auto Vec3::operator-(const Vec3& other) const -> Vec3 {
	return this->sub(other);
}
inline constexpr auto Vec3::operator*(const f32& other) const -> Vec3 {
	return this->mult(other);
}
inline constexpr auto Vec3::operator/(const f32& other) const -> Vec3 {
	return this->div(other);
}

inline constexpr auto Vec3::operator+=(const Vec3& other) -> Vec3& {
	*this = *this + other;
	return *this;
}
inline constexpr auto Vec3::operator-=(const Vec3& other) -> Vec3& {
	*this = *this - other;
	return *this;
}
inline constexpr auto Vec3::operator*=(const f32& other) -> Vec3& {
	*this = *this * other;
	return *this;
}
inline constexpr auto Vec3::operator/=(const f32& other) -> Vec3& {
	*this = *this / other;
	return *this;
}

inline constexpr auto Vec3::operator==(const Vec3& other) -> bool {
	return ((x == other.x) && (y == other.y) && (z == other.z));
}
inline constexpr auto Vec3::operator!=(const Vec3& other) -> bool {
	return !(*this == other);
}

inline constexpr auto Vec3::add(const Vec3 o) const -> Vec3 {
	return Vec3(x + o.x, y + o.y, z + o.z);
}
inline constexpr auto Vec3::sub(const Vec3 o) const -> Vec3 {
	return Vec3(x - o.x, y - o.y, z - o.z);
}
inline constexpr auto Vec3::mult(const f32 o) const -> Vec3 {
	return Vec3(x * o, y * o, z * o);
}
inline constexpr auto Vec3::div(const f32 o) const -> Vec3 {
	return Vec3(x / o, y / o, z / o);
}
inline constexpr auto Vec3::get_length() const -> f32 {
	f32 dot = this->dot(*this);
	return dot * dot;
}
inline constexpr auto Vec3::dot(const Vec3& o) const -> f32 {
	return x * o.x + y * o.y + z * o.z;
}
inline constexpr auto Vec3::cross(const Vec3& o) const -> Vec3 {
	return Vec3(
		y * o.z - z * o.y,
		z * o.x - x * o.z,
		x * o.y - y * o.x
	);
}
inline constexpr auto Vec3::get_normal() const -> Vec3 {
	const f32& length = this->get_length();
	return Vec3(x / length, y / length, z / length);
}