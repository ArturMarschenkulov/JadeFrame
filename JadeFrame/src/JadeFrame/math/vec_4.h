#pragma once
class Vec3;
class Vec4 {
public:
	Vec4();
	Vec4(const float x, const float y, const float z, const float w);
	Vec4(const Vec3& vec3, const float w);
	explicit Vec4(const float scalar);

	auto operator=(const Vec4& other)->Vec4&;
	auto operator+(const Vec4& other) const->Vec4;
	auto operator-(const Vec4& other) const->Vec4;
	auto operator*(const float& other) const->Vec4;
	auto operator/(const float& other) const->Vec4;

	auto operator+=(const Vec4& other)->Vec4&;
	auto operator-=(const Vec4& other)->Vec4&;
	auto operator*=(const float& other)->Vec4&;
	auto operator/=(const float& other)->Vec4&;

	auto operator==(const Vec4& other) -> bool;
	auto operator!=(const Vec4& other) -> bool;


	auto add(const Vec4 o) const->Vec4;
	auto sub(const Vec4 o) const->Vec4;
	auto mult(const float o) const->Vec4;
	auto div(const float o) const->Vec4;
	auto get_length() const->float;
	auto dot(const Vec4& o) const -> float;
	auto get_normal() const->Vec4;
public:
	float x;
	float y;
	float z;
	float w;
};


class Quaternion {
public:

public:
	float a;
	float i;
	float j;
	float k;
};