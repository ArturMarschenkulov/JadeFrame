#pragma once
#include <string>
#include "Vec3.h"

class Vec4 {
public:
	Vec4();
	Vec4(float scalar);
	Vec4(float x, float y, float z, float w);
	Vec4(float x, float y, float z);
	Vec4(Vec3 vec3);
	Vec4(Vec3 vec3, float w);
public:
	float x;
	float y;
	float z;
	float w;
};