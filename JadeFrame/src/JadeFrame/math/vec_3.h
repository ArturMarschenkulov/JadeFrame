#pragma once
#include "JadeFrame/defines.h"
class Vec4;
class Vec2;
class Vec3 {
public:
	Vec3();
	Vec3(const f32 x, const f32 y, const f32 z);
	Vec3(const Vec2& vec, const f32 z);
	Vec3(const Vec3& vec);
	explicit Vec3(const f32 scalar);
	explicit Vec3(const Vec4& vec);

	auto operator=(const Vec3& other)->Vec3&;
	auto operator+(const Vec3& other) const->Vec3;
	auto operator-(const Vec3& other) const->Vec3;
	auto operator*(const f32& other) const->Vec3;
	auto operator/(const f32& other) const->Vec3;

	auto operator+=(const Vec3& other)->Vec3&;
	auto operator-=(const Vec3& other)->Vec3&;
	auto operator*=(const f32& other)->Vec3&;
	auto operator/=(const f32& other)->Vec3&;

	auto operator==(const Vec3& other) -> bool;
	auto operator!=(const Vec3& other) -> bool;


	auto add(const Vec3 o) const->Vec3;
	auto sub(const Vec3 o) const->Vec3;
	auto mult(const f32 o) const->Vec3;
	auto div(const f32 o) const->Vec3;
	auto get_length() const->f32;
	auto dot(const Vec3& o) const -> f32;
	auto cross(const Vec3& o) const->Vec3;
	auto get_normal() const->Vec3;
public:
	f32 x;
	f32 y;
	f32 z;
};
