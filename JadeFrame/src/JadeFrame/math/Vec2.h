#pragma once
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
