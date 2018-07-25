#pragma once

#include "fx_base.h"

namespace fx {

class FXManager {
  public:
	FXManager();
	~FXManager();

	void saveDefs() const;

	void simulate(float time_delta);

	const auto &particleDefs() const { return m_particle_defs; }
	const auto &emitterDefs() const { return m_emitter_defs; }
	const auto &systemDefs() const { return m_system_defs; }

	bool valid(ParticleDefId) const;
	bool valid(EmitterDefId) const;
	bool valid(ParticleSystemDefId) const;

	const ParticleDef &operator[](ParticleDefId) const;
	const EmitterDef &operator[](EmitterDefId) const;
	const ParticleSystemDef &operator[](ParticleSystemDefId) const;

	bool valid(ParticleSystemId) const;
	bool alive(ParticleSystemId) const;
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

	vector<DrawParticle> genQuads();

  private:
	void addDefaultDefs();
	void simulate(ParticleSystem &, float time_delta);
	void initialize(const ParticleSystemDef &, ParticleSystem &);
	SubSystemContext ssctx(ParticleSystem &, int);

	vector<ParticleDef> m_particle_defs;
	vector<EmitterDef> m_emitter_defs;
	vector<ParticleSystemDef> m_system_defs;

	// TODO: add simple statistics: num particles, instances, etc.
	vector<ParticleSystem> m_systems;
	int m_spawn_clock = 0, m_random_seed = 0;
};
}
