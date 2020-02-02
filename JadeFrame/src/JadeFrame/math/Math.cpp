#include "Math.h"

float toRadian(float degrees) {
	return degrees*( M_PI / 180.0f );

}
float toDegree(float radians) {
	return radians*( 180.0f / M_PI );
}
