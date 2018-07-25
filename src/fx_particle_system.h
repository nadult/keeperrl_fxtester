#pragma once

#include "fx_base.h"
#include "fx_color.h"
#include "fx_curve.h"
#include "fx_rect.h"
#include "fx_tag_id.h"
#include <limits.h>

RICH_ENUM(EmissionSourceType, point, rect, sphere);

namespace fx {

// Identifies a particluar particle system instance
class ParticleSystemId {
  public:
	ParticleSystemId() : m_index(-1) {}
	ParticleSystemId(int index, int spawn_time) : m_index(index), m_spawn_time(spawn_time) {
		PASSERT(index >= 0 && spawn_time >= 0);
	}

	bool validIndex() const { return m_index >= 0; }
	explicit operator bool() const { return validIndex(); }

	operator int() const { return m_index; }
	int index() const { return m_index; }
	int spawnTime() const { return m_spawn_time; }

  private:
	int m_index;
	int m_spawn_time;
};

struct ParticleDef {
	void serialize(IArchive &, unsigned int);
	void serialize(OArchive &, unsigned int) const;

	// Defines spawned particle life in seconds for given AT
	Curve<float> life;

	// These curves modify particle appearance based on
	// particle time
	Curve<float> alpha;
	Curve<float> size;
	Curve<float> slowdown;
	Curve<FVec3> color;

	vector<Curve<float>> scalar_curves;
	vector<Curve<FVec3>> color_curves;

	IVec2 texture_tiles = IVec2(1, 1);
	string texture_name;
	string name;
};

// TODO: move to separate file
class EmissionSource {
  public:
	using Type = EmissionSourceType;

	EmissionSource() : m_type(Type::point) {}
	EmissionSource(FVec2 pos) : m_type(Type::point), m_pos(pos) {}
	EmissionSource(FRect rect)
		: m_type(Type::rect), m_pos(rect.center()), m_param(rect.size() * 0.5f) {}
	EmissionSource(FVec2 pos, float radius)
		: m_type(Type::sphere), m_pos(pos), m_param(radius, 0.0f) {}

	Type type() const { return m_type; }
	FVec2 pos() const { return m_pos; }
	FRect rect() const {
		PASSERT(m_type == Type::rect);
		return {m_pos - m_param, m_pos + m_param};
	}
	float sphereRadius() const {
		PASSERT(m_type == Type::sphere);
		return m_param.x;
	}

	// TODO(opt): sample multiple points at once
	FVec2 sample(RandomGen &) const;

	void serialize(IArchive &, unsigned int);
	void serialize(OArchive &, unsigned int) const;

  private:
	FVec2 m_pos, m_param;
	Type m_type;
};

// Emiterem mogą też być cząsteczki innego subsystemu ?
struct EmitterDef {
	void serialize(IArchive &, unsigned int);
	void serialize(OArchive &, unsigned int) const;

	EmissionSource source;
	Curve<float> frequency; // particles per second
	Curve<float> strength_min, strength_max;
	Curve<float> direction, direction_spread; // in radians
	Curve<float> rotation_speed_min, rotation_speed_max;

	vector<Curve<float>> scalar_curves;
	vector<Curve<FVec3>> color_curves;

	float initial_spawn_count = 0.0f;

	// TODO: zamiast częstotliwości możemy mieć docelową ilość cząsteczek
	// (danego rodzaju?) w danym momencie
	// TODO: całkowanie niektórych krzywych?

	string name;
};

using AnimateParticleFunc = void (*)(AnimationContext &, Particle &);
using DrawParticleFunc = void (*)(DrawContext &, const Particle &, DrawParticle &);

// Returns number of particles to emit
// Fractionals will be accumulated over time
using PrepareEmissionFunc = float (*)(AnimationContext &, EmissionState &);
using EmitParticleFunc = void (*)(AnimationContext &, EmissionState &, Particle &);

void defaultAnimateParticle(AnimationContext &, Particle &);
float defaultPrepareEmission(AnimationContext &, EmissionState &);
void defaultEmitParticle(AnimationContext &, EmissionState &, Particle &);
void defaultDrawParticle(DrawContext &, const Particle &, DrawParticle &);

struct ParticleSystemDef {
	struct SubSystem {
		SubSystem(ParticleDefId pdef, EmitterDefId edef) : particle_id(pdef), emitter_id(edef) {}

		ParticleDefId particle_id;
		EmitterDefId emitter_id;

		AnimateParticleFunc animate_func = defaultAnimateParticle;
		PrepareEmissionFunc prepare_func = defaultPrepareEmission;
		EmitParticleFunc emit_func = defaultEmitParticle;
		DrawParticleFunc draw_func = defaultDrawParticle;

		int max_active_particles = INT_MAX;
		int max_total_particles = INT_MAX;
	};

	void serialize(IArchive &, unsigned int);
	void serialize(OArchive &, unsigned int) const;

	const SubSystem &operator[](int ssid) const { return subsystems[ssid]; }
	SubSystem &operator[](int ssid) { return subsystems[ssid]; }

	vector<SubSystem> subsystems;
	float anim_length = 1.0f;
	bool is_looped = false;
	string name;
};

struct Particle {
	float particleTime() const { return life / max_life; }

	// TODO: quantize members ? It may give the particles a pixelated feel
	FVec2 pos, movement, size = FVec2(1.0);
	float life = 0.0f, max_life = 1.0f;
	float rot = 0.0f, rot_speed = 0.0f;
	SVec2 tex_tile;
};

struct DrawParticle {
	// TODO: compress it somehow?
	std::array<FVec2, 4> positions;
	std::array<FVec2, 4> tex_coords;
	IColor color;
	int particle_def_id;
};

struct ParticleSystem {
	struct SubSystem {
		vector<Particle> particles;
		float emission_fract = 0.0f;
		int random_seed = 123;
		int total_particles = 0;
	};

	struct Params {
		float scalar[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		FVec3 color[2] = {FVec3(1.0), FVec3(1.0)};
	};

	ParticleSystem(FVec2 pos, ParticleSystemDefId, int spawn_time, int num_subsystems);

	void kill();

	const SubSystem operator[](int ssid) const { return subsystems[ssid]; }
	SubSystem &operator[](int ssid) { return subsystems[ssid]; }

	vector<SubSystem> subsystems;
	Params params;
	FVec2 pos;

	ParticleSystemDefId def_id;
	int spawn_time;

	float anim_time = 0.0f;
	bool is_dead = false;
};

struct SubSystemContext {
	const ParticleSystem &ps;
	const ParticleSystem::SubSystem &ss;

	const ParticleSystemDef &psdef;
	const ParticleSystemDef::SubSystem &ssdef;

	const ParticleDef &pdef;
	const EmitterDef &edef;

	const int ssid;
};

struct AnimationContext : public SubSystemContext {
	float uniformSpread(float spread);
	float uniform(float min, float max);
	int randomSeed();
	SVec2 randomTexTile();

	RandomGen rand;
	const float anim_time, norm_anim_time;
	const float time_delta, inv_time_delta;
};

struct DrawContext : public SubSystemContext {
	FVec2 inv_tex_tile;

	array<FVec2, 4> quadCorners(FVec2 pos, FVec2 size, float rotation) const;
	array<FVec2, 4> texQuadCorners(SVec2 tex_tile) const;
};

struct EmissionState {
	float rot_min, rot_max;
	float strength_min, strength_max;

	float max_life;
	float angle, angle_spread;
	float rot_speed_min, rot_speed_max; // TODO: change to value, value_var or value_spread

	float var[128];
};
}
