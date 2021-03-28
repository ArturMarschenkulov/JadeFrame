#pragma once
#include <string>
//#include "Vec3.h"
class Vec3;
class Vec4 {
public:
	Vec4();
	Vec4(const float scalar);
	Vec4(const float x, const float y, const float z, const float w);
	Vec4(const Vec3& vec3, const float w);
public:
	float x;
	float y;
	float z;
	float w;
};
