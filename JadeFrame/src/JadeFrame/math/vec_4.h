#pragma once
#include "JadeFrame/defines.h"

namespace JadeFrame {

class Vec3;
class Vec4 {
public:
	Vec4();
	Vec4(const f32 x, const f32 y, const f32 z, const f32 w);
	Vec4(const Vec3& vec3, const f32 w);
	explicit Vec4(const f32 scalar);

	auto operator=(const Vec4& other)->Vec4&;
	auto operator+(const Vec4& other) const->Vec4;
	auto operator-(const Vec4& other) const->Vec4;
	auto operator*(const f32& other) const->Vec4;
	auto operator/(const f32& other) const->Vec4;

	auto operator+=(const Vec4& other)->Vec4&;
	auto operator-=(const Vec4& other)->Vec4&;
	auto operator*=(const f32& other)->Vec4&;
	auto operator/=(const f32& other)->Vec4&;

	auto operator==(const Vec4& other) -> bool;
	auto operator!=(const Vec4& other) -> bool;


	auto add(const Vec4 o) const->Vec4;
	auto sub(const Vec4 o) const->Vec4;
	auto mult(const f32 o) const->Vec4;
	auto div(const f32 o) const->Vec4;
	auto get_length() const->f32;
	auto dot(const Vec4& o) const->f32;
	auto get_normal() const->Vec4;
public:
	f32 x;
	f32 y;
	f32 z;
	f32 w;
};


class Quaternion {
public:

public:
	f32 a;
	f32 i;
	f32 j;
	f32 k;
};

}