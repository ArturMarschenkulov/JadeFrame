#pragma once
class Vec4;
class Vec2;
class Vec3 {
public:
	Vec3();
	Vec3(const float x, const float y, const float z);
	Vec3(const Vec2& vec, const float z);
	Vec3(const Vec3& vec);
	explicit Vec3(const float scalar);
	explicit Vec3(const Vec4& vec);

	auto operator=(const Vec3& other)->Vec3&;
	auto operator+(const Vec3& other) const->Vec3;
	auto operator-(const Vec3& other) const->Vec3;
	auto operator*(const float& other) const->Vec3;
	auto operator/(const float& other) const->Vec3;

	auto operator+=(const Vec3& other)->Vec3&;
	auto operator-=(const Vec3& other)->Vec3&;
	auto operator*=(const float& other)->Vec3&;
	auto operator/=(const float& other)->Vec3&;

	auto operator==(const Vec3& other) -> bool;
	auto operator!=(const Vec3& other) -> bool;


	auto add(const Vec3 o) const->Vec3;
	auto sub(const Vec3 o) const->Vec3;
	auto mult(const float o) const->Vec3;
	auto div(const float o) const->Vec3;
	auto get_length() const->float;
	auto dot(const Vec3& o) const -> float;
	auto cross(const Vec3& o) const->Vec3;
	auto get_normal() const->Vec3;
public:
	float x;
	float y;
	float z;
};
