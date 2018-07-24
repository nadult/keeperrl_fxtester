#pragma once

#include "curve.h"

#include "tag_id.h"

static constexpr int default_tile_size = 24;

// Najpierw chcemy uzyskać proste efekty:
// - emiter który jest w stanie emitować ileś tam cząsteczek na sekundę
// - cząsteczka która może zmieniać kolor, ma kierunek lotu, zmieniającą się alfę i czas życia
// - mamy też cały system cząstek (najczęściej w ramach pojedyńczego kafla)
//   - może być parametryzowany: parametry zmieniają trochę konfigurację danego systemu
//
// Szablony emiterów i cząsteczek umożliwiają robienie generycznych efektów (z pomocą ścieżek)
// ale chyba chcielibyśmy też mieć możliwość pisania specjalnych klas do konkretnych efektów
//
// Na początku możemy zrobić generyczny system (podobny do tego co mieliśmy w XLIB) a później
// jeśli generyczny system nie wystarczy, to dodać specjalizacje;
//
// Nad specjalizacjami zastanawiamy się w kontekście konkretnego efektu jaki chcemy uzyskać

// AT: animation time:
// PT: particle time: particle.life / particle.max_life

struct ParticleDef;
struct EmitterDef;
struct ParticleSystemDef;

using ParticleDefId = TagId<ParticleDef>;
using EmitterDefId = TagId<EmitterDef>;
using ParticleSystemDefId = TagId<ParticleSystemDef>;

// Identifies a particluar particle system instance
// Can be invalid (instances with time can die)
class ParticleSystemId {
  public:
	ParticleSystemId() : m_index(-1) {}
	ParticleSystemId(int index, int spawn_time) : m_index(index), m_spawn_time(spawn_time) {
		CHECK(index >= 0 && spawn_time >= 0);
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
	// Defines spawned particle life in seconds for given AT
	Curve<float> life;

	// These curves modify particle appearance based on
	// particle time
	Curve<float> alpha;
	Curve<float> size;
	Curve<float> slowdown;
	Curve<float> attract_bottom; // TODO: jakoś lepiej to zrobić
	Curve<FVec3> color;

	IVec2 texture_tiles = IVec2(1, 1);
	string texture_name;
	string name;
};

// Na jakie sposoby możemy definiować różne wartości:
// - stała wartość
// - krzywa w różny sposób interpolowana, regularna lub nie
// - dwie krzywe (min / max)
// - ... ?
// - niektóre parametry powinny być opcjonalne ?

// Różne kształty emitera;
// Emiterem mogą też być cząsteczki innego subsystemu ?
struct EmitterDef {
	Curve<float> frequency; // particles per second
	Curve<float> strength_min, strength_max;
	Curve<float> direction, direction_spread; // in radians
	Curve<float> rotation_speed_min, rotation_speed_max;

	float initial_spawn_count = 0.0f;

	// TODO: zamiast częstotliwości możemy mieć docelową ilość cząsteczek
	// (danego rodzaju?) w danym momencie
	// TODO: całkowanie niektórych krzywych?

	string name;
};

struct ParticleSystemDef {
	struct SubSystem {
		SubSystem(ParticleDefId pdef, EmitterDefId edef) : particle_id(pdef), emitter_id(edef) {}

		ParticleDefId particle_id;
		EmitterDefId emitter_id;

		int max_active_particles = INT_MAX;
		int max_total_particles = INT_MAX;
	};

	const SubSystem operator[](int ssid) const { return subsystems[ssid]; }
	SubSystem &operator[](int ssid) { return subsystems[ssid]; }

	std::vector<SubSystem> subsystems;
	float anim_length = 1.0f;
	bool is_looped = false;
	string name;
};

struct Particle {
	float particleTime() const { return life / max_life; }

	FVec2 pos, movement;
	float size, life, max_life;
	float rot, rot_speed;
	SVec2 tex_tile;
};

struct ParticleSystem {
	struct SubSystem {
		std::vector<Particle> particles;
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

	// TODO(OPT): this should be small vector?
	std::vector<SubSystem> subsystems;
	Params params;
	FVec2 pos;

	ParticleSystemDefId def_id;
	int spawn_time;

	float anim_time = 0.0f;
	bool is_dead = false;
};
