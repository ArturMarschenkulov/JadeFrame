#pragma once
#include <cmath>
class Vec4;
class Vec2;
class Vec3 {
public:
	Vec3();
	Vec3(const float scalar);
	Vec3(const float x, const float y, const float z);
	Vec3(const Vec2& vec, const float z);
	explicit Vec3(const Vec4& vec);
	Vec3(const Vec3& vec);

	auto operator=(const Vec3& other) -> Vec3& {
		this->x = other.x;
		this->y = other.y;
		this->z = other.z;
		return *this;
	}
	auto operator+(const Vec3& other) const -> Vec3 {
		return { this->x + other.x, this->y + other.y, this->z + other.z };
	}
	auto operator-(const Vec3& other) const -> Vec3{
		return { this->x - other.x, this->y - other.y, this->z - other.z };
	}
	auto operator*(const float& other) const -> Vec3 {
		return { this->x * other, this->y * other, this->z * other };
	}
	auto operator/(const float& other) const -> Vec3 {
		return { this->x / other, this->y / other, this->z / other };
	}

	auto operator+=(const Vec3& other) -> Vec3& {
		*this = *this + other;
		return *this;
	}
	auto operator-=(const Vec3& other) -> Vec3& {
		*this = *this - other;
		return *this;
	}
	auto operator*=(const float& other) -> Vec3& {
		*this = *this * other;
		return *this;
	}
	auto operator/=(const float& other) -> Vec3& {
		*this = *this / other;
		return *this;
	}

	auto operator==(const Vec3& other) -> bool{
		return ((x == other.x) && (y == other.y) && (z == other.z));
	}
	auto operator!=(const Vec3& other) -> bool {
		return !(*this == other);
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
		float length = static_cast<float>(sqrt(x * x + y * y + z * z));
		if(length == 0) {
			length = 10000;
		}
		return Vec3(x / length, y / length, z / length);
	}
	auto normalize() const -> Vec3 {
		float length = static_cast<float>(sqrt(x * x + y * y + z * z));
		return Vec3(x / length, y / length, z / length);
	}
public:
	float x;
	float y;
	float z;
};
