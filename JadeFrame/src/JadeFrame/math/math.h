#pragma once
#undef min
#undef max
#include "mat_4.h"
#include "vec.h"
#include "JadeFrame/defines.h"

#define M_PI 3.14159265359f


namespace JadeFrame {
auto to_radians(f32 degrees)->f32;
auto to_degrees(f32 radians)->f32;

template <typename T>
auto is_power_of_two(T value) -> bool {
	return ((value & (value - 1)) == (T)0);
}

}

//#include <type_traits>
//#include <limits>
//#include <array>
//#include <cmath>
//#include <iostream>
//
//template<std::size_t... Is>
//struct Seq {
//};
//
//template<std::size_t N, std::size_t... Is>
//struct GenSeq : GenSeq<N - 1, N, Is...> {
//};
//
//template<std::size_t... Is>
//struct GenSeq<0, Is...> : Seq<Is...> {
//};
//
//namespace math {
//template<typename T> constexpr T pi = 3.14159265358979323846264338327L;
//template<typename T> constexpr T two_pi = 6.28318530717958647692528676656L;
//template<typename T> constexpr T half_pi = pi<T> *0.5;
//
//constexpr static f64 pi_v = pi<f64>;
//constexpr static f64 two_pi_v = two_pi<f64>;
//constexpr static f64 half_pi_v = half_pi<f64>;
//
//template<class T, class dcy = std::decay_t<T>>
//constexpr inline auto inverse(T value) -> std::enable_if_t<std::is_floating_point<T>::value, dcy> {
//	return (value == 0) ? 0.0 : 1.0 / value;
//}
//constexpr inline auto factorial(std::intmax_t const& n) -> f64 {
//	if (n == 0) {
//		return 1;
//	}
//	f64 result = n;
//	for (std::intmax_t i = n - 1; i > 0; --i) {
//		result *= i;
//	}
//	return result;
//}
//constexpr inline auto max_factorial() -> std::size_t {
//	std::size_t i = 0;
//	f64 d = 0;
//	while ((d = factorial(i)) < std::numeric_limits<f32>::max()) {
//		++i;
//	}
//	return i;
//}
//
//template<class Base, std::size_t N>
//class TrigCeoffs {
//	using T = typename Base::ValueType;
//	using ArrayType = std::array<T, N>;
//
//	template<std::size_t ... NS>
//	constexpr static inline auto _coeffs(Seq<NS ...>) -> ArrayType {
//		return { {Base::coeff(NS) ...} };
//	}
//public:
//	constexpr static ArrayType coeffs = _coeffs(GenSeq<N>{});
//};
//template<class Base, std::size_t N>
//constexpr typename TrigCeoffs<Base, N>::ArrayType TrigCeoffs<Base, N>::coeffs;
//
//
//template<class Base, std::size_t N, class dcy = std::decay_t<typename Base::ValueType>>
//constexpr auto _sincos(typename Base::ValueType x) noexcept -> std::enable_if_t<std::is_floating_point<dcy>::value, dcy> {
//	using C = TrigCeoffs<Base, N>;
//
//	if (std::isnan(x) && std::numeric_limits<dcy>::has_quiet_NaN) {
//		return static_cast<dcy>(std::numeric_limits<dcy>::quiet_NaN());
//	} else if (std::isinf(x) && std::numeric_limits<dcy>::has_infinity) {
//		return static_cast<dcy>(std::numeric_limits<dcy>::infinity());
//	} else {
//		dcy result = 0.0;//result accumulator
//		//do input range mapping
//		dcy _x = Base::range_reduce(x);
//		//taylor series
//		{
//			const dcy x_2 = _x * _x; //store x^2
//			dcy pow = Base::initial_condition(_x);
//			for (auto&& cf : C::coeffs) {
//				result += cf * pow;
//				pow *= x_2;
//			}
//		}
//		return result;
//	}
//}
//namespace detail {
//template<class T>
//struct _Sin {
//	using ValueType = T;
//	constexpr static inline auto coeff(std::size_t n)noexcept -> T {
//		return (n % 2 ? 1 : -1) * inverse(factorial((2 * n) - 1));
//	}
//	constexpr static inline auto range_reduce(T x)noexcept -> T {
//		T _x = x;
//		_x += math::pi<T>;
//		_x -= static_cast<std::size_t>(_x / math::two_pi<T>) * math::two_pi<T>;
//		_x -= math::pi<T>;
//		return _x;
//	}
//	constexpr static inline auto initial_condition(T x)noexcept -> T {
//		return x;
//	}
//	constexpr static inline auto default_N()noexcept -> std::size_t {
//		return 16;
//	}
//};
//}
//template<class T, std::size_t N = detail::_Sin<T>::default_N()>
//constexpr inline auto sin(T x)noexcept -> std::decay_t<T> {
//	return _sincos<detail::_Sin<T>, N>(x);
//}
//
//}
//
////int main(int argc, char** argv) {
////	f64 phs = 0;
////	f64 stp = math::two_pi_v / 100.0;
////	for (int i = 0; i < 100; ++i) {
////		std::cout << math::sin(phs) << std::endl;
////		phs += stp;
////	}
////	return 0;
////}
