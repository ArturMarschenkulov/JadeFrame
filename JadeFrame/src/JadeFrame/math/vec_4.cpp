#include "vec_4.h"
#include "vec_3.h"
#include <cmath>

Vec4::Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
Vec4::Vec4(const f32 scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
Vec4::Vec4(const f32 x, const f32 y, const f32 z, const f32 w) : x(x), y(y), z(z), w(w) {}
Vec4::Vec4(const Vec3& vec3, const f32 w): x(vec3.x), y(vec3.y), z(vec3.z), w(w) {}


auto Vec4::operator=(const Vec4& other) -> Vec4& {
	this->x = other.x;
	this->y = other.y;
	this->z = other.z;
	return *this;
}
auto Vec4::operator+(const Vec4& other) const -> Vec4 {
	return this->add(other);
}
auto Vec4::operator-(const Vec4& other) const -> Vec4 {
	return this->sub(other);
}
auto Vec4::operator*(const f32& other) const -> Vec4 {
	return this->mult(other);
}
auto Vec4::operator/(const f32& other) const -> Vec4 {
	return this->div(other);
}

auto Vec4::operator+=(const Vec4& other) -> Vec4& {
	*this = *this + other;
	return *this;
}
auto Vec4::operator-=(const Vec4& other) -> Vec4& {
	*this = *this - other;
	return *this;
}
auto Vec4::operator*=(const f32& other) -> Vec4& {
	*this = *this * other;
	return *this;
}
auto Vec4::operator/=(const f32& other) -> Vec4& {
	*this = *this / other;
	return *this;
}

auto Vec4::operator==(const Vec4& other) -> bool {
	return ((x == other.x) && (y == other.y) && (z == other.z));
}
auto Vec4::operator!=(const Vec4& other) -> bool {
	return !(*this == other);
}

auto Vec4::add(const Vec4 o) const -> Vec4 {
	return Vec4(x + o.x, y + o.y, z + o.z, w + o.w);
}
auto Vec4::sub(const Vec4 o) const -> Vec4 {
	return Vec4(x - o.x, y - o.y, z - o.z, w - o.w);
}
auto Vec4::mult(const f32 o) const -> Vec4 {
	return Vec4(x * o, y * o, z * o, w * o);
}
auto Vec4::div(const f32 o) const -> Vec4 {
	return Vec4(x / o, y / o, z / o, w / o);
}
auto Vec4::get_length() const -> f32 {
	return std::sqrt(this->dot(*this));
}
auto Vec4::dot(const Vec4& o) const -> f32 {
	return x * o.x + y * o.y + z * o.z;
}
auto Vec4::get_normal() const -> Vec4 {
	f32 length = this->get_length();
	return Vec4(x / length, y / length, z / length, w / length);
}
