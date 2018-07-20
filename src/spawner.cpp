#include "spawner.h"

#include "fx_manager.h"

Spawner::Spawner(Type type, int2 tile_pos, ParticleSystemDefId def_id)
	: type(type), tile_pos(tile_pos), def_id(def_id) {}

void Spawner::update(FXManager &manager) {
	if(is_dead)
		return;

	if(manager.dead(instance_id)) {
		if(spawn_count > 0 && type == Type::single) {
			is_dead = true;
			return;
		}

		spawn_count++;
		instance_id = manager.addSystem(def_id, float2(tile_pos) * default_tile_size);
	}
}

void Spawner::remove(FXManager &manager) { manager.kill(instance_id); }
