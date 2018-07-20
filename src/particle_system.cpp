#include "particle_system.h"

#include <fwk/math/random.h>

ParticleSystem::ParticleSystem(float2 pos, ParticleSystemDefId def_id, int spawn_time,
							   int num_subsystems)
	: pos(pos), def_id(def_id), spawn_time(spawn_time), subsystems(num_subsystems) {}

void ParticleSystem::kill() {
	subsystems.clear();
	is_dead = true;
}
