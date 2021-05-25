#include "vec_2.h"
#include "vec_3.h"
#include "vec_4.h"
#include <cmath>

Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
Vec3::Vec3(const f32 scalar) : x(scalar), y(scalar), z(scalar) {}
Vec3::Vec3(const f32 x, const f32 y, const f32 z) : x(x), y(y), z(z) {}
Vec3::Vec3(const Vec2& vec2, f32 z) : x(vec2.x), y(vec2.y), z(z) {}
Vec3::Vec3(const Vec4& vec4) : x(vec4.x), y(vec4.y), z(vec4.z) {}
Vec3::Vec3(const Vec3& vec) : x(vec.x), y(vec.y), z(vec.z) {}


auto Vec3::operator=(const Vec3& other) -> Vec3& {
	this->x = other.x;
	this->y = other.y;
	this->z = other.z;
	return *this;
}
auto Vec3::operator+(const Vec3& other) const -> Vec3 {
	return this->add(other);
}
auto Vec3::operator-(const Vec3& other) const -> Vec3 {
	return this->sub(other);
}
auto Vec3::operator*(const f32& other) const -> Vec3 {
	return this->mult(other);
}
auto Vec3::operator/(const f32& other) const -> Vec3 {
	return this->div(other);
}

auto Vec3::operator+=(const Vec3& other) -> Vec3& {
	*this = *this + other;
	return *this;
}
auto Vec3::operator-=(const Vec3& other) -> Vec3& {
	*this = *this - other;
	return *this;
}
auto Vec3::operator*=(const f32& other) -> Vec3& {
	*this = *this * other;
	return *this;
}
auto Vec3::operator/=(const f32& other) -> Vec3& {
	*this = *this / other;
	return *this;
}

auto Vec3::operator==(const Vec3& other) -> bool {
	return ((x == other.x) && (y == other.y) && (z == other.z));
}
auto Vec3::operator!=(const Vec3& other) -> bool {
	return !(*this == other);
}

auto Vec3::add(const Vec3 o) const -> Vec3 {
	return Vec3(x + o.x, y + o.y, z + o.z);
}
auto Vec3::sub(const Vec3 o) const -> Vec3 {
	return Vec3(x - o.x, y - o.y, z - o.z);
}
auto Vec3::mult(const f32 o) const -> Vec3 {
	return Vec3(x * o, y * o, z * o);
}
auto Vec3::div(const f32 o) const -> Vec3 {
	return Vec3(x / o, y / o, z / o);
}
auto Vec3::get_length() const -> f32 {
	return std::sqrt(this->dot(*this));
}
auto Vec3::dot(const Vec3& o) const -> f32 {
	return x * o.x + y * o.y + z * o.z;
}
auto Vec3::cross(const Vec3& o) const -> Vec3 {
	return Vec3(
		y * o.z - z * o.y,
		z * o.x - x * o.z,
		x * o.y - y * o.x
	);
}
auto Vec3::get_normal() const -> Vec3 {
	f32 length = this->get_length();
	return Vec3(x / length, y / length, z / length);
}