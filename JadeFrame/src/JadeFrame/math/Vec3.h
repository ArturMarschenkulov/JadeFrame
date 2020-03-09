#pragma once
#include <cmath>
class Vec4;
class Vec2;
class Vec3 {
public:
	Vec3();
	Vec3(float scalar);
	Vec3(float x, float y, float z);
	Vec3(float x, float y);
	Vec3(Vec2 vec);
	Vec3(Vec4 vec);
	Vec3(Vec3& vec);
	Vec3(const Vec3& vec);
	auto operator=(const Vec3& other) -> Vec3& {
		this->x = other.x;
		this->y = other.y;
		this->z = other.z;
		return *this;
	}
	auto operator+(const Vec3& other) const -> Vec3 {
		Vec3 result;
		result.x = x + other.x;
		result.y = y + other.y;
		result.z = z + other.z;
		return result;
	}
	auto operator-(const Vec3& other) -> Vec3{
		Vec3 result;
		result.x = x - other.x;
		result.y = y - other.y;
		result.z = z - other.z;
		return result;
	}
	auto operator*(const float& other) -> Vec3 {
		Vec3 result;
		result.x = x * other;
		result.y = y * other;
		result.z = z * other;
		return result;
	}
	auto operator+=(const Vec3& other) -> Vec3& {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	auto operator-=(const Vec3& other) -> Vec3& {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	auto operator==(const Vec3& other) -> bool{
		return (x == other.x && y == other.y && z == other.z);
	}
	auto operator!=(const Vec3& other) -> bool {
		return (x != other.x || y == other.y || z != other.z);
	}

	auto cross(const Vec3& other) const -> Vec3 {
		Vec3 result = Vec3(
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		);
		return result;
	}

	auto normalize() -> Vec3 {
		float length = (float)sqrt(x * x + y * y + z * z);
		if(length == 0) {
			length = 10000;
		}
		return Vec3(x / length, y / length, z / length);
	}
	auto normalize() const -> Vec3 {
		float length = (float)sqrt(x * x + y * y + z * z);
		return Vec3(x / length, y / length, z / length);
	}
public:
	float x;
	float y;
	float z;
};