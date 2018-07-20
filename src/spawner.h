#pragma once

#include "base.h"

DEFINE_ENUM(SpawnerType, single, repeated);

struct Spawner {
	using Type = SpawnerType;

	// TODO: dodatkowe parametry
	Spawner(Type type, int2 tile_pos, int system_id);

	void update(ParticleManager &);
	void remove(ParticleManager &);

	Type type;
	int2 tile_pos;
	int system_id;
	Maybe<int> instance_id;
	bool is_dead = false;
};
