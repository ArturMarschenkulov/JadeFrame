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

	Vec3& normalize() {
		float length = ( float ) sqrt(x * x + y * y + z * z);
		if ( length > 0 ) {
			x /= length;
			y /= length;
			z /= length;
		}
		return *this;
	}
public:
	float x;
	float y;
	float z;
};