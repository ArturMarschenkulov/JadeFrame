#include "vec_4.h"
#include "vec_3.h"
#include <cmath>

Vec4::Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
Vec4::Vec4(const float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
Vec4::Vec4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}
Vec4::Vec4(const Vec3& vec3, const float w): x(vec3.x), y(vec3.y), z(vec3.z), w(w) {}


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
auto Vec4::operator*(const float& other) const -> Vec4 {
	return this->mult(other);
}
auto Vec4::operator/(const float& other) const -> Vec4 {
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
auto Vec4::operator*=(const float& other) -> Vec4& {
	*this = *this * other;
	return *this;
}
auto Vec4::operator/=(const float& other) -> Vec4& {
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
auto Vec4::mult(const float o) const -> Vec4 {
	return Vec4(x * o, y * o, z * o, w * o);
}
auto Vec4::div(const float o) const -> Vec4 {
	return Vec4(x / o, y / o, z / o, w / o);
}
auto Vec4::get_length() const -> float {
	return std::sqrt(this->dot(*this));
}
auto Vec4::dot(const Vec4& o) const -> float {
	return x * o.x + y * o.y + z * o.z;
}
auto Vec4::get_normal() const -> Vec4 {
	float length = this->get_length();
	return Vec4(x / length, y / length, z / length, w / length);
}
