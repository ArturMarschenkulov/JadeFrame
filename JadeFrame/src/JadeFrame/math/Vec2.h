#pragma once
#include <cmath>
class Vec3;
class Vec2 {
public:
	Vec2();
	Vec2(float scalar);
	Vec2(float x, float y);
	Vec2(Vec3);
	Vec2 operator+(Vec2 other) {
		return { this->x + other.x, this->y + other.y };
	}
	Vec2 operator-(Vec2 other) {
		return { this->x - other.x, this->y - other.y };
	}
	Vec2 operator*(float other) {
		return { this->x * other, this->y * other };
	}
	Vec2 operator/(float other) {
		return { this->x / other, this->y / other };
	}
	Vec2& operator+=(Vec2 other) {
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
	Vec2& operator-=(Vec2 other) {
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}
	Vec2& operator*=(float other) {
		this->x *= other;
		this->y *= other;
		return *this;
	}
	Vec2& operator/=(float other) {
		this->x /= other;
		this->y /= other;
		return *this;
	}

	void rotate(float rad) {
		float xx = this->x;
		float yy = this->y;
		this->x = xx * std::cos(rad) - yy * std::sin(rad);
		this->y = xx * std::sin(rad) + yy * std::cos(rad);
		//return *this;
	}
	float getMagnitude() {
		return std::sqrt(this->x * this->x + this->y * this->y);
	}
	float getMagnitudeSquared() {
		return this->x * this->x + this->y * this->y;
	}
	void setMagnitude(float n) {
		this->normalize();
		*this = *this * n;
	}
	void setLimit(float max) {
		float mSq = this->getMagnitudeSquared();
		if ( mSq > max * max ) {
			*this = ( *this ) / std::sqrt(mSq);
			*this = ( *this ) / max;
		}
	}
	float getDistance(Vec2 other) {
		Vec2 r = *this - other;
		return std::sqrt(r.x * r.x + r.y * r.y);
	}
	void normalize() {
		auto len = this->getMagnitude();
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