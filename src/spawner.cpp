#include "spawner.h"

#include "fx_manager.h"

Spawner::Spawner(Type type, fwk::int2 tile_pos, ParticleSystemDefId def_id)
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
		auto spawn_pos = (fwk::float2(tile_pos) + fwk::float2(0.5f)) * default_tile_size;
		instance_id = manager.addSystem(def_id, {spawn_pos.x, spawn_pos.y});
	}
}

void Spawner::kill(FXManager &manager) {
	manager.kill(instance_id);
	is_dead = true;
}
