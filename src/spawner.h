#pragma once

#include "particle_system.h"

DEFINE_ENUM(SpawnerType, single, repeated);

struct Spawner {
	using Type = SpawnerType;

	// TODO: dodatkowe parametry
	Spawner(Type type, int2 tile_pos, ParticleSystemDefId);

	void update(FXManager &);
	void remove(FXManager &);

	int2 tile_pos;
	ParticleSystemDefId def_id;
	ParticleSystemId instance_id;
	int spawn_count = 0;
	bool is_dead = false;
	Type type;
};
