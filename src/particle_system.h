#pragma once

#include "base.h"
#include "curve.h"

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

// TODO: lepsze nazewnictwo...

// AT: animation time:
// PT: particle time: particle.life / particle.max_life

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
		int emitter_id;
		int particle_id;
	};
	vector<SubSystem> subsystems;
	float anim_length;
	// TODO: co się dzieje jak się kończy animacja? kasujemy instancję, czy wyłączamy emisję?
};

struct ParticleInstance {
	float particleTime() const { return life / max_life; }

	float2 pos, movement;
	float size, life, max_life;
	float rot, rot_speed;
};

// TODO: better name to differentiate from system ?
struct SubSystemInstance {
	vector<ParticleInstance> particles;
	float emission_fract = 0.0f;
	int random_seed = 123;
};

struct ParticleSystemInstance {
	ParticleSystemInstance(float2 pos, int system_id);

	void clear();

	// TODO: this should be small vector?
	vector<SubSystemInstance> subsystems;
	float2 pos;
	int system_id;
	float anim_time = 0.0f;
	bool is_dead = false;
};

struct RenderQuad {
	array<float2, 4> positions;
	array<float2, 4> tex_coords;
	array<FColor, 4> colors;
	int particle_def_id;
};

class ParticleManager {
  public:
	int addInstance(int system_id, float2 pos);
	void simulate(float time_delta);
	void addDefaults();

	const auto &particleDefs() const { return m_particle_defs; }
	const auto &emitterDefs() const { return m_emitter_defs; }
	const auto &systemDefs() const { return m_system_defs; }
	const auto &instances() const { return m_instances; }

	vector<RenderQuad> genQuads() const;

	// TODO: otypowane indeksy ?

  private:
	void simulate(ParticleSystemInstance &, float time_delta);
	void initialize(ParticleSystemInstance &);

	vector<ParticleDef> m_particle_defs;
	vector<EmitterDef> m_emitter_defs;
	vector<ParticleSystemDef> m_system_defs;

	// TODO: tutaj przydałby się IndexedVector?
	// (żeby mozna było trzymać indeksy konkretnych instancji)
	vector<ParticleSystemInstance> m_instances;
};
