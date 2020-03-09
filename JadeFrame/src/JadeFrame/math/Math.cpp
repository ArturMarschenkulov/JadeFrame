#include "Math.h"

auto to_radians(float degrees) -> float {
	return degrees*( M_PI / 180.0f );

}
auto to_degrees(float radians) -> float {
	return radians*( 180.0f / M_PI );
}
