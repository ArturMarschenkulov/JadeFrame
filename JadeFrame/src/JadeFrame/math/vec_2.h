#pragma once
class Vec3;
class Vec2 {
public:
	Vec2();
	Vec2(const float scalar);
	Vec2(const float x, const float y);
	explicit Vec2(const Vec3&);


	auto operator==(const Vec2 & other) const -> bool;
	auto operator!=(const Vec2 & other) const -> bool;

	auto operator+(const Vec2& other) const->Vec2;
	auto operator-(const Vec2& other) const->Vec2;
	auto operator*(const float& other) const->Vec2;
	auto operator/(const float& other) const->Vec2;

	auto operator+=(const Vec2& other)->Vec2&;
	auto operator-=(const Vec2& other)->Vec2&;
	auto operator*=(const float& other)->Vec2&;
	auto operator/=(const float& other)->Vec2&;


public:
	union {
		struct {
			float x;
			float y;
		};
		struct {
			float w;
			float h;
		};
		struct {
			float width;
			float height;
		};
	};

};