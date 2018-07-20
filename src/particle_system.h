#pragma once

#include "base.h"
#include "curve.h"

#include "tag_id.h"

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

using ParticleDefId = TagId<ParticleDef>;
using EmitterDefId = TagId<EmitterDef>;
using ParticleSystemDefId = TagId<ParticleSystemDef>;

// Identifies a particluar particle system instance
// Can be invalid (instances with time can die)
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
	// Defines spawned particle life in seconds for given AT
	Curve<float> life;

	// These curves modify particle appearance based on
	// particle time
	Curve<float> alpha;
	Curve<float> size;
	Curve<float3> color;

	string texture_name;
	string name;
};

// Różne kształty emitera;
// Emiterem mogą też być cząsteczki innego subsystemu ?
struct EmitterDef {
	Curve<float> frequency; // particles per second
	Curve<float> strength;
	Curve<float> direction, direction_spread; // in radians

	// TODO: zamiast częstotliwości możemy mieć docelową ilość cząsteczek
	// (danego rodzaju?) w danym momencie
	// TODO: całkowanie niektórych krzywych?

	string name;
};

struct ParticleSystemDef {
	struct SubSystem {
		ParticleDefId particle_id;
		EmitterDefId emitter_id;
	};
	vector<SubSystem> subsystems;
	float anim_length;
	string name;
	// TODO: co się dzieje jak się kończy animacja? kasujemy instancję, czy wyłączamy emisję?
};

struct Particle {
	float particleTime() const { return life / max_life; }

	float2 pos, movement;
	float size, life, max_life;
	float rot, rot_speed;
};

struct ParticleSystem {
	struct SubSystem {
		vector<Particle> particles;
		float emission_fract = 0.0f;
		int random_seed = 123;
	};

	ParticleSystem(float2 pos, ParticleSystemDefId, int spawn_time, int num_subsystems);

	void kill();

	// TODO: this should be small vector?
	vector<SubSystem> subsystems;
	float2 pos;

	ParticleSystemDefId def_id;
	int spawn_time;

	float anim_time = 0.0f;
	bool is_dead = false;
};
