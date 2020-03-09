#pragma once
#include <cmath>
class Vec3;
class Vec2 {
public:
	Vec2();
	Vec2(float scalar);
	Vec2(float x, float y);
	Vec2(Vec3);
	auto operator+(Vec2 other) -> Vec2{
		return { this->x + other.x, this->y + other.y };
	}
	auto operator-(Vec2 other) -> Vec2 {
		return { this->x - other.x, this->y - other.y };
	}
	auto operator*(float other) -> Vec2 {
		return { this->x * other, this->y * other };
	}
	auto operator/(float other) -> Vec2 {
		return { this->x / other, this->y / other };
	}
	auto operator+=(Vec2 other) -> Vec2& {
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
	auto operator-=(Vec2 other) -> Vec2& {
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}
	auto operator*=(float other) -> Vec2& {
		this->x *= other;
		this->y *= other;
		return *this;
	}
	auto operator/=(float other) -> Vec2& {
		this->x /= other;
		this->y /= other;
		return *this;
	}

	auto rotate(float rad) -> void {
		float xx = this->x;
		float yy = this->y;
		this->x = xx * std::cos(rad) - yy * std::sin(rad);
		this->y = xx * std::sin(rad) + yy * std::cos(rad);
		//return *this;
	}
	auto get_magnitude() -> float {
		return std::sqrt(this->x * this->x + this->y * this->y);
	}
	auto get_magnitude_squared() -> float {
		return this->x * this->x + this->y * this->y;
	}
	auto set_magnitude(float n) -> void {
		this->normalize();
		*this = *this * n;
	}
	auto set_limit(float max) -> void {
		float mSq = this->get_magnitude_squared();
		if ( mSq > max * max ) {
			*this = ( *this ) / std::sqrt(mSq);
			*this = ( *this ) / max;
		}
	}
	auto get_distance(Vec2 other) -> float 
	{
		Vec2 r = *this - other;
		return std::sqrt(r.x * r.x + r.y * r.y);
	}
	auto normalize() -> void {
		auto len = this->get_magnitude();
		if ( len != 0 ) {
			*this = *this * ( 1 / len );
		}
	}


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