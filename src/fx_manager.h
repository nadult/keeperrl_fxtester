#pragma once

#include "particle_system.h"
#include "renderer.h" // TODO: only for Color

struct RenderQuad {
	// TODO: compress it somehow
	std::array<FVec2, 4> positions;
	std::array<FVec2, 4> tex_coords;
	Color color;
	int particle_def_id;
};

class FXManager {
  public:
	FXManager();
	~FXManager();

	void simulate(float time_delta);

	const auto &particleDefs() const { return m_particle_defs; }
	const auto &emitterDefs() const { return m_emitter_defs; }
	const auto &systemDefs() const { return m_system_defs; }

	bool valid(ParticleDefId id) const { return id < (int)m_particle_defs.size(); }
	bool valid(EmitterDefId id) const { return id < (int)m_emitter_defs.size(); }
	bool valid(ParticleSystemDefId id) const { return id < (int)m_system_defs.size(); }

	const ParticleDef &operator[](ParticleDefId id) const {
		CHECK(valid(id));
		return m_particle_defs[id];
	}
	const EmitterDef &operator[](EmitterDefId id) const {
		CHECK(valid(id));
		return m_emitter_defs[id];
	}
	const ParticleSystemDef &operator[](ParticleSystemDefId id) const {
		CHECK(valid(id));
		return m_system_defs[id];
	}

	bool valid(ParticleSystemId) const;
	bool dead(ParticleSystemId) const; // invalid ids will be dead
	void kill(ParticleSystemId);

	// id cannot be invalid
	ParticleSystem &get(ParticleSystemId);
	const ParticleSystem &get(ParticleSystemId) const;

	ParticleSystemId addSystem(ParticleSystemDefId, FVec2 pos);

	ParticleDefId addDef(ParticleDef);
	EmitterDefId addDef(EmitterDef);
	ParticleSystemDefId addDef(ParticleSystemDef);

	vector<ParticleSystemId> aliveSystems() const;
	const auto &systems() const { return m_systems; }
	auto &systems() { return m_systems; }

	std::vector<RenderQuad> genQuads() const;

  private:
	void addDefaultDefs();
	void simulate(ParticleSystem &, float time_delta);
	void initialize(ParticleSystem &);

	std::vector<ParticleDef> m_particle_defs;
	std::vector<EmitterDef> m_emitter_defs;
	std::vector<ParticleSystemDef> m_system_defs;

	// TODO: add simple statistics: num particles, instances, etc.

	// TODO: tutaj przydałby się IndexedVector?
	// (żeby mozna było trzymać indeksy konkretnych instancji)
	//
	// TODO: ref-countowane instancje ? możemy też zrobić podobnie jak
	// we FreeFT: każda instancja ma też licznik spawnu;
	vector<ParticleSystem> m_systems;
	RandomGen m_random;
	int m_spawn_clock = 0;
};
