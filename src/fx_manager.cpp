#include "fx_manager.h"

#include "fcolor.h"
#include "fmath.h"
#include "frect.h"

FXManager::FXManager() { addDefaultDefs(); }
FXManager::~FXManager() = default;

void FXManager::simulate(ParticleSystem &ps, float time_delta) {
	auto &def = (*this)[ps.def_id];

	float norm_anim_pos = ps.anim_time / def.anim_length;

	// Animating live particles
	for(int ssid = 0; ssid < (int)def.subsystems.size(); ssid++) {
		auto &pdef = m_particle_defs[def.subsystems[ssid].particle_id];
		auto &ssinst = ps.subsystems[ssid];
		float inv_time_delta = 1.0f / time_delta;

		for(auto &pinst : ssinst.particles) {
			float ptime = pinst.particleTime();
			float slowdown = 1.0f / (1.0f + pdef.slowdown.sample(ptime));
			float attract_bottom = pdef.attract_bottom.sample(ptime);

			pinst.pos += pinst.movement * time_delta;
			pinst.rot += pinst.rot_speed * time_delta;
			if(slowdown < 1.0f) {
				float factor = pow(slowdown, time_delta);
				pinst.movement *= factor;
				pinst.rot_speed *= factor;
			}
			if(attract_bottom > 0.0f) {
				float attract_min = 5.0f, attract_max = 10.0f;
				if(pinst.pos.y < attract_min) {
					float dist = attract_min - pinst.pos.y;
					pinst.movement += FVec2(0.0f, dist * attract_bottom);
				}
			}
			pinst.life += time_delta;
		}
	}

	// Removing dead particles
	for(auto &ssinst : ps.subsystems)
		for(int n = 0; n < (int)ssinst.particles.size(); n++) {
			auto &pinst = ssinst.particles[n];
			if(pinst.life > pinst.max_life) {
				pinst = ssinst.particles.back();
				ssinst.particles.pop_back();
				n--;
			}
		}

	// Emitting new particles
	for(int ssid = 0; ssid < (int)def.subsystems.size(); ssid++) {
		auto &ssdef = def.subsystems[ssid];
		auto &pdef = m_particle_defs[ssdef.particle_id];
		auto &edef = m_emitter_defs[ssdef.emitter_id];
		auto &ssinst = ps.subsystems[ssid];

		RandomGen rand;
		rand.init(ssinst.random_seed);

		float max_life = pdef.life.sample(norm_anim_pos);
		float freq = edef.frequency.sample(norm_anim_pos);
		float emission = freq * time_delta + ssinst.emission_fract;
		int num_particles = int(emission);
		ssinst.emission_fract = emission - float(num_particles);

		float angle = edef.direction.sample(norm_anim_pos);
		float angle_spread = edef.direction_spread.sample(norm_anim_pos);

		float strength_min = edef.strength_min.sample(norm_anim_pos);
		float strength_max = edef.strength_max.sample(norm_anim_pos);

		float rot_speed_min = edef.rotation_speed_min.sample(norm_anim_pos);
		float rot_speed_max = edef.rotation_speed_max.sample(norm_anim_pos);

		int max_particles = min(ssdef.max_active_particles - (int)ssinst.particles.size(),
								ssdef.max_total_particles - ssinst.total_particles);
		num_particles = min(num_particles, max_particles);

		for(int n = 0; n < num_particles; n++) {
			Particle new_inst;
			new_inst.pos = FVec2();
			float pangle;
			if(angle_spread < fconstant::pi)
				pangle = angle + rand.getDouble(-angle_spread, angle_spread);
			else
				pangle = rand.getDouble(0.0f, fconstant::pi * 2.0f);
			FVec2 pdir = angleToVector(pangle);
			float strength = rand.getDouble(strength_min, strength_max);
			float rot_speed = rand.getDouble(rot_speed_min, rot_speed_max);
			new_inst.movement = pdir * strength;
			new_inst.rot = rand.getDouble(0.0f, fconstant::pi * 2.0f);
			new_inst.rot_speed = rot_speed * strength;
			new_inst.life = 0.0f;
			new_inst.max_life = max_life;

			if(!(pdef.texture_tiles == IVec2(1, 1))) {
				IVec2 tex_tile(rand.get(pdef.texture_tiles.x - 1),
							   rand.get(pdef.texture_tiles.y - 1));
				new_inst.tex_tile = SVec2(tex_tile);
			}
			ssinst.particles.emplace_back(new_inst);
			ssinst.total_particles++;
		}
		ssinst.random_seed = rand.getLL() % 1973257861;
	}

	ps.anim_time += time_delta;
	if(ps.anim_time > def.anim_length)
		ps.kill();
}

void FXManager::simulate(float delta) {
	for(auto &inst : m_systems)
		if(!inst.is_dead)
			simulate(inst, delta);
}

std::vector<RenderQuad> FXManager::genQuads() const {
	std::vector<RenderQuad> out;

	for(auto &ps : m_systems) {
		if(ps.is_dead)
			continue;
		auto &def = (*this)[ps.def_id];

		for(int ssid = 0; ssid < (int)def.subsystems.size(); ssid++) {
			int part_def_id = def.subsystems[ssid].particle_id;
			auto &pdef = m_particle_defs[part_def_id];
			auto &ssinst = ps.subsystems[ssid];

			FVec2 inv_tex_tile = vinv(FVec2(pdef.texture_tiles));
			for(auto &pinst : ssinst.particles) {
				float ptime = pinst.particleTime();

				FVec2 pos = pinst.pos + ps.pos;
				FVec2 size(pdef.size.sample(ptime) * 0.5f);
				float alpha = pdef.alpha.sample(ptime);

				FRect tex_rect(FVec2(1));
				if(!(pdef.texture_tiles == IVec2(1, 1)))
					tex_rect = (tex_rect + FVec2(pinst.tex_tile)) * inv_tex_tile;

				auto corners = FRect(pos - size, pos + size).corners();
				FColor color(pdef.color.sample(ptime), alpha);
				for(auto &corner : corners)
					corner = rotateVector(corner - pos, pinst.rot) + pos;
				out.emplace_back(
					RenderQuad{corners, tex_rect.corners(), Color(color), part_def_id});
			}
		}
	}

	return out;
}

bool FXManager::valid(ParticleSystemId id) const {
	return id >= 0 && id < m_systems.size() && m_systems[id].spawn_time == id.spawnTime();
}

bool FXManager::dead(ParticleSystemId id) const { return !valid(id) || m_systems[id].is_dead; }

void FXManager::kill(ParticleSystemId id) {
	if(!dead(id))
		m_systems[id].kill();
}

ParticleSystem &FXManager::get(ParticleSystemId id) {
	CHECK(valid(id));
	return m_systems[id];
}

const ParticleSystem &FXManager::get(ParticleSystemId id) const {
	CHECK(valid(id));
	return m_systems[id];
}

ParticleSystemId FXManager::addSystem(ParticleSystemDefId def_id, FVec2 pos) {
	auto &def = (*this)[def_id];

	for(int n = 0; n < m_systems.size(); n++)
		if(m_systems[n].is_dead) {
			if(m_systems[n].spawn_time == m_spawn_clock)
				m_spawn_clock++;

			m_systems[n] = {pos, def_id, m_spawn_clock, (int)def.subsystems.size()};
			initialize(def, m_systems[n]);
			return ParticleSystemId(n, m_spawn_clock);
		}

	m_systems.emplace_back(pos, def_id, m_spawn_clock, (int)def.subsystems.size());
	initialize(def, m_systems.back());
	return ParticleSystemId(m_systems.size() - 1, m_spawn_clock);
}

void FXManager::initialize(const ParticleSystemDef &def, ParticleSystem &ps) {
	for(int ssid = 0; ssid < (int)ps.subsystems.size(); ssid++) {
		auto &ss = ps.subsystems[ssid];
		ss.random_seed = m_random.getLL() % 1973257861;
		ss.emission_fract = (*this)[def.subsystems[ssid].emitter_id].initial_spawn_count;
	}
	// TODO: initial particles
}

vector<ParticleSystemId> FXManager::aliveSystems() const {
	vector<ParticleSystemId> out;
	out.reserve(m_systems.size());

	for(int n = 0; n < m_systems.size(); n++)
		if(!m_systems[n].is_dead)
			out.emplace_back(ParticleSystemId(n, m_systems[n].spawn_time));

	return out;
}

ParticleDefId FXManager::addDef(ParticleDef def) {
	m_particle_defs.emplace_back(std::move(def));
	return ParticleDefId(m_particle_defs.size() - 1);
}

EmitterDefId FXManager::addDef(EmitterDef def) {
	m_emitter_defs.emplace_back(std::move(def));
	return EmitterDefId(m_emitter_defs.size() - 1);
}

ParticleSystemDefId FXManager::addDef(ParticleSystemDef def) {
	m_system_defs.emplace_back(std::move(def));
	return ParticleSystemDefId(m_system_defs.size() - 1);
}
