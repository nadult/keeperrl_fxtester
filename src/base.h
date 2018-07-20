#pragma once

#include <fwk/gfx/color.h>
#include <fwk/math/box_iter.h>
#include <fwk/math/random.h>
#include <fwk/math/triangle.h>
#include <fwk/sys/assert.h>
#include <fwk/sys/unique_ptr.h>
#include <fwk/sys/xml.h>
#include <fwk_profile.h>

using namespace fwk;

struct ParticleDef;
struct EmitterDef;
struct ParticleSystemDef;

struct Particle;
struct ParticleSystem;

class FXManager;
struct RenderQuad;

static constexpr int default_tile_size = 24;

struct RenderQuad {
	array<float2, 4> positions;
	array<float2, 4> tex_coords;
	FColor color;
	int particle_def_id;
};
