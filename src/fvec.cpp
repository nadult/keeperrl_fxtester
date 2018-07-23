#include "fvec.h"

#include <math.h>

float vectorToAngle(const float2 &normalized_vec) {
	float ang = std::acos(normalized_vec.x);
	return normalized_vec.y < 0 ? 2.0f * fconstant::pi - ang : ang;
}

float2 angleToVector(float radians) {
	auto sc = sincos(radians);
	return float2(sc.second, sc.first);
}

float2 rotateVector(const float2 &vec, float radians) {
	auto sc = sincos(radians);
	return float2(sc.second * vec.x - sc.first * vec.y, sc.second * vec.y + sc.first * vec.x);
}
