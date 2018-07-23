#pragma once

#include <fwk/math/box.h>
#include <fwk_vector.h>

#define FWK_VEC2_CONVERSIONS                                                                       \
	vec2(const fwk::vec2<T> &rhs) : x(rhs.x), y(rhs.y) {}                                          \
	operator fwk::vec2<T>() const { return {x, y}; }

#define FWK_VEC3_CONVERSIONS                                                                       \
	vec3(const fwk::vec3<T> &rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}                                \
	operator fwk::vec3<T>() const { return {x, y, z}; }

#include "frect.h"
#include "fvec.h"

namespace fx_tester {

class ImGuiWrapper;

using namespace fwk;
};
