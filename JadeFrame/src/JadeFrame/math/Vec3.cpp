#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"

Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
Vec3::Vec3(float scalar) : x(scalar), y(scalar), z(scalar) {}
Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
Vec3::Vec3(float x, float y) : x(x), y(y), z(0.0f) {}
Vec3::Vec3(Vec2 vec2) : x(vec2.x), y(vec2.y), z(0.0f) {}
Vec3::Vec3(Vec4 vec4) : x(vec4.x), y(vec4.y), z(vec4.z) {}
Vec3::Vec3(Vec3& vec) : x(vec.x), y(vec.y), z(vec.z) {}
Vec3::Vec3(const Vec3& vec) : x(vec.x), y(vec.y), z(vec.z) {}

