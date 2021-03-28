#include "vec_2.h"
#include "vec_3.h"
#include "vec_4.h"

Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
Vec3::Vec3(const float scalar) : x(scalar), y(scalar), z(scalar) {}
Vec3::Vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
//Vec3::Vec3(const float x, const float y) : x(x), y(y), z(0.0f) {}
Vec3::Vec3(const Vec2& vec2, float z) : x(vec2.x), y(vec2.y), z(z) {}
Vec3::Vec3(const Vec4& vec4) : x(vec4.x), y(vec4.y), z(vec4.z) {}
Vec3::Vec3(const Vec3& vec) : x(vec.x), y(vec.y), z(vec.z) {}

