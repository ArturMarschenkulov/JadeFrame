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

	Vec3& operator+(const Vec3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vec3& operator-(const Vec3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	Vec3 cross(const Vec3& other) const {
		return Vec3(
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		);
	}
	Vec3 normalize() const {
		float length = (float)sqrt(x * x + y * y + z * z);
		return Vec3(x / length, y / length, z / length);
	}
public:
	float x;
	float y;
	float z;
};