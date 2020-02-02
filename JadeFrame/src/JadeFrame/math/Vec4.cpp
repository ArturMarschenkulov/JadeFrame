#include "Vec4.h"

Vec4::Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
Vec4::Vec4(float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
Vec4::Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
Vec4::Vec4(float x, float y, float z) : x(x), y(y), z(z), w(0.0f) {}
Vec4::Vec4(Vec3 vec3) : x(vec3.x), y(vec3.y), z(vec3.z), w(0.0f) {}
Vec4::Vec4(Vec3 vec3, float w): x(vec3.x), y(vec3.y), z(vec3.z), w(w) {}
