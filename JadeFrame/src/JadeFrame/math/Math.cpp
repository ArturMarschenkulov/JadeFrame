#include "pch.h"
#include "math.h"

namespace JadeFrame {
auto to_radians(f32 degrees) -> f32 {
	//return degrees * (M_PI / 180.0f);
	return degrees * static_cast<f32>(0.01745329251994329576923690768489);
}
auto to_degrees(f32 radians) -> f32 {
	//return radians * (180.0f / M_PI);
	return radians * static_cast<f32>(57.295779513082320876798154814105);
}

static auto is_prime(u32 x) -> bool {
	if (x >= 2) {
		for (u32 i = 0; i <= x; i++) {
			if (x % i == 1 || x % i == x) {
				return true;
			}
		}
	}
	return false;

	if (x < 2) return false;
	for (u32 i = 2; i * i <= x; i++) {
		if (x % i == 0) return false;
	}
	return true;
}


//
//#include <cmath>
//struct Rational {
//	int numerator;
//	int denomitor;
//
//
//	auto operator+(const Rational& o) const -> Rational {
//		return _add(o);
//	}
//	auto operator-(const Rational& o) const -> Rational {
//		return _sub(o);
//	}
//	auto operator*(const Rational& o) const-> Rational {
//		return _mult(o);
//	}
//	auto operator/(const Rational& o) const -> Rational {
//		return _div(o);
//	}
//
//private:
//	auto _add(const Rational& o) const -> Rational {
//		auto a = numerator * o.denomitor + denomitor * o.numerator;
//		auto b = denomitor * o.denomitor;
//		return { a, b };
//	}
//	auto _sub(const Rational& o) const -> Rational {
//		auto a = numerator * o.denomitor - denomitor * o.numerator;
//		auto b = denomitor * o.denomitor;
//		return { a, b };
//	}
//	auto _mult(const Rational& o) const -> Rational {
//		auto a = numerator * o.numerator;
//		auto b = denomitor * o.denomitor;
//		return { a, b };
//	}
//
//	auto _div(const Rational& o) const -> Rational {
//		auto a = numerator * o.denomitor;
//		auto b = denomitor * o.numerator;
//		return { a, b };
//	}
//	auto _equal(const Rational& o) const -> bool {
//		return numerator * o.denomitor == denomitor * o.numerator;
//	}
//	auto _inverse() const -> Rational {
//		auto a = -numerator;
//		auto b = denomitor;
//		return { a, b };
//	}
//	auto _reciprocal() const -> Rational {
//		auto a = denomitor;
//		auto b = numerator;
//		return { a, b };
//	}
//	auto _pow(int e) const -> Rational {
//		auto a = std::pow(denomitor, e);
//		auto b = std::pow(numerator, e);
//		return { a, b };
//	}
//};




//static auto line_closest_point(const Vec2& a, const Vec2& b, const Vec2& p) -> Vec2 {
//    Vec2 ap = p - a;
//    Vec2 ab_dir = b - a;
//    f32 ab_len = sqrtf(ab_dir.x * ab_dir.x + ab_dir.y * ab_dir.y);
//    ab_dir *= 1.0f / ab_len;
//    f32 dot = ap.x * ab_dir.x + ap.y * ab_dir.y;
//    if (dot < 0.0f)
//        return a;
//    if (dot > ab_len)
//        return b;
//    return a + ab_dir * dot;
//}
//
//static auto triangle_contains_point(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& p) -> bool {
//    bool b1 = ((p.x - b.x) * (a.y - b.y) - (p.y - b.y) * (a.x - b.x)) < 0.0f;
//    bool b2 = ((p.x - c.x) * (b.y - c.y) - (p.y - c.y) * (b.x - c.x)) < 0.0f;
//    bool b3 = ((p.x - a.x) * (c.y - a.y) - (p.y - a.y) * (c.x - a.x)) < 0.0f;
//    return ((b1 == b2) && (b2 == b3));
//}
}