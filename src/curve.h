#pragma once

#include "base.h"

DEFINE_ENUM(InterpolationType, linear, cosine, quadratic, cubic);

template <class T> T interpCosine(const T &a, const T &b, float x) {
	return lerp(a, b, (1.0f - std::cos(x * fconstant::pi)) * 0.5f);
}

template <class T> T interpQuadratic(const T &v0, const T &v1, const T &v2, const T &v3, float x) {
	// TODO: to chyba nie jest poprawne
	return v1 * (1 - x) + v2 * x + (v2 - v3 + v1 - v0) * (1 - x) * x;
}

template <class T> T interpCubic(const T &y0, const T &y1, const T &y2, const T &y3, float mu) {
	// Source: http://paulbourke.net/miscellaneous/
	auto mu_sq = mu * mu;
	//	auto a0 = y3 - y2 - y0 + y1;
	//	auto a1 = y0 - y1 - a0;
	//	auto a2 = y2 - y0;
	//	auto a3 = y1;

	auto a0 = -0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3;
	auto a1 = y0 - 2.5 * y1 + 2 * y2 - 0.5 * y3;
	auto a2 = -0.5 * y0 + 0.5 * y2;
	auto a3 = y1;

	return a0 * mu * mu_sq + a1 * mu_sq + a2 * mu + a3;
}

// TODO: RegularCurve<>

template <class T> struct Curve {
  public:
	Curve(vector<float> keys, vector<T> values, InterpolationType);

	// Position is always within range: <0, 1>
	T sample(float position) const;

	vector<float> m_keys;
	vector<T> m_values;
	InterpolationType m_interp; // TODO: this doesn't have to be here
};

extern template struct Curve<float>;
extern template struct Curve<float2>;
extern template struct Curve<float3>;
