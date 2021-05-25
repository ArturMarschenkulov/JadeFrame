#pragma once
#include "JadeFrame/defines.h"
class Vec3;
class Vec2 {
public:
	Vec2();
	Vec2(const f32 x, const f32 y);
	explicit Vec2(const Vec3&);
	explicit Vec2(const f32 scalar);


	auto operator==(const Vec2 & other) const -> bool;
	auto operator!=(const Vec2 & other) const -> bool;

	auto operator+(const Vec2& other) const->Vec2;
	auto operator-(const Vec2& other) const->Vec2;
	auto operator*(const f32& other) const->Vec2;
	auto operator/(const f32& other) const->Vec2;

	auto operator+=(const Vec2& other)->Vec2&;
	auto operator-=(const Vec2& other)->Vec2&;
	auto operator*=(const f32& other)->Vec2&;
	auto operator/=(const f32& other)->Vec2&;


public:
	union {
		struct {
			f32 x;
			f32 y;
		};
		struct {
			f32 w;
			f32 h;
		};
		struct {
			f32 width;
			f32 height;
		};
	};

};