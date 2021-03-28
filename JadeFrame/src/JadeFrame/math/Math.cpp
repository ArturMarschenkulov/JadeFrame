#include "Math.h"

auto to_radians(float degrees) -> float {
	return degrees * (M_PI / 180.0f);

}
auto to_degrees(float radians) -> float {
	return radians * (180.0f / M_PI);
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