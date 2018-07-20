#include "spawner.h"

#include "particle_system.h"

Spawner::Spawner(Type type, int2 tile_pos, int system_id)
	: type(type), tile_pos(tile_pos), system_id(system_id) {}

void Spawner::update(ParticleManager &manager) {
	if(is_dead)
		return;

	if(instance_id) {
		auto &inst = manager.instances()[*instance_id];
		if(inst.is_dead) { // TODO: proper cleanup
			instance_id = none;
			if(type == Type::single) {
				is_dead = true;
				return;
			}
		}
	} else {
		instance_id = manager.addInstance(system_id, float2(tile_pos) * default_tile_size);
	}
}

void Spawner::remove(ParticleManager &manager) {
	// TODO: proper removal
	if(instance_id)
		manager.remove(*instance_id);
}
