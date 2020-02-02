#include "Vec2.h"
#include "Vec3.h"

Vec2::Vec2() : x(0.0f), y(0.0f) {}
Vec2::Vec2(float scalar) : x(scalar), y(scalar) {}
Vec2::Vec2(float x, float y) : x(x), y(y){}
Vec2::Vec2(Vec3 vec3) : x(vec3.x), y(vec3.y) {}
