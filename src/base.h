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
struct ParticleInstance;
struct SubSystemInstance;

struct RenderQuad;

class ParticleManager;

static constexpr int default_tile_size = 24;
