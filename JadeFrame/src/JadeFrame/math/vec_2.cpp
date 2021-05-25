#include "vec_2.h"
#include "vec_3.h"

Vec2::Vec2() : x(0.0f), y(0.0f) {
}
Vec2::Vec2(const f32 scalar) : x(scalar), y(scalar) {
}
Vec2::Vec2(const f32 x, const f32 y) : x(x), y(y) {
}
Vec2::Vec2(const Vec3& vec3) : x(vec3.x), y(vec3.y) {
}


auto Vec2::operator==(const Vec2& other) const -> bool {
	return ((x == other.x) && (y == other.y));
}
auto Vec2::operator!=(const Vec2& other) const -> bool {
	return !(*this == other);
}
auto Vec2::operator+(const Vec2& other) const -> Vec2 {
	return { this->x + other.x, this->y + other.y };
}

auto Vec2::operator-(const Vec2& other) const -> Vec2 {
	return { this->x - other.x, this->y - other.y };
}
auto Vec2::operator*(const f32& other) const -> Vec2 {
	return { this->x * other, this->y * other };
}
auto Vec2::operator/(const f32& other) const-> Vec2 {
	return { this->x / other, this->y / other };
}
auto Vec2::operator+=(const Vec2& other) -> Vec2& {
	*this = *this + other;
	return *this;
}
auto Vec2::operator-=(const Vec2& other) -> Vec2& {
	*this = *this - other;
	return *this;
}
auto Vec2::operator*=(const f32& other) -> Vec2& {
	*this = *this * other;
	return *this;
}
auto Vec2::operator/=(const f32& other) -> Vec2& {
	*this = *this / other;
	return *this;
}

