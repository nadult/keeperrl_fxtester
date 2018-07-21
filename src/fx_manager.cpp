#include "fx_manager.h"

FXManager::FXManager() { addDefaultDefs(); }
FXManager::~FXManager() = default;

void FXManager::simulate(ParticleSystem &ps, float time_delta) {
	auto &def = (*this)[ps.def_id];

	float norm_anim_pos = ps.anim_time / def.anim_length;

	// Animating live particles
	for(int ssid = 0; ssid < def.subsystems.size(); ssid++) {
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
				FRect bottom_rect(-12, 5, 10, 12);
				auto closest_pos = bottom_rect.closestPoint(pinst.pos);
				float dist = distance(closest_pos, pinst.pos);

				if(dist > 0.01f)
					pinst.movement += (closest_pos - pinst.pos) * attract_bottom;
			}
			pinst.life += time_delta;
		}
	}

	// Removing dead particles
	for(auto &ssinst : ps.subsystems)
		for(int n = 0; n < ssinst.particles.size(); n++) {
			auto &pinst = ssinst.particles[n];
			if(pinst.life > pinst.max_life) {
				pinst = ssinst.particles.back();
				ssinst.particles.pop_back();
				n--;
			}
		}

	// Emitting new particles
	for(int ssid = 0; ssid < def.subsystems.size(); ssid++) {
		auto &ssdef = def.subsystems[ssid];
		auto &pdef = m_particle_defs[ssdef.particle_id];
		auto &edef = m_emitter_defs[ssdef.emitter_id];
		auto &ssinst = ps.subsystems[ssid];

		// TODO: random z keepera
		Random rand(ssinst.random_seed);

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

		num_particles = min(num_particles, ssdef.max_active_particles - ssinst.particles.size(),
							ssdef.max_total_particles - ssinst.total_particles);

		for(int n = 0; n < num_particles; n++) {
			Particle new_inst;
			new_inst.pos = float2();
			float pangle;
			if(angle_spread < fconstant::pi)
				pangle = angle + rand.uniform(-angle_spread, angle_spread);
			else
				pangle = rand.uniform(0.0f, fconstant::pi * 2.0f);
			float2 pdir = angleToVector(pangle);
			float strength = rand.uniform(strength_min, strength_max);
			float rot_speed = rand.uniform(rot_speed_min, rot_speed_max);
			new_inst.movement = pdir * strength;
			new_inst.rot = rand.uniform(0.0f, fconstant::pi * 2.0f);
			new_inst.rot_speed = rot_speed * strength;
			new_inst.life = 0.0f;
			new_inst.max_life = max_life;

			if(pdef.texture_tiles != int2(1, 1)) {
				int2 tex_tile(rand.uniform(pdef.texture_tiles.x),
							  rand.uniform(pdef.texture_tiles.y));
				new_inst.tex_tile = tex_tile;
			}
			ssinst.particles.emplace_back(new_inst);
			ssinst.total_particles++;
		}
		ssinst.random_seed = rand();
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

vector<RenderQuad> FXManager::genQuads() const {
	vector<RenderQuad> out;

	for(auto &ps : m_systems) {
		if(ps.is_dead)
			continue;
		auto &def = (*this)[ps.def_id];

		for(int ssid = 0; ssid < def.subsystems.size(); ssid++) {
			int part_def_id = def.subsystems[ssid].particle_id;
			auto &pdef = m_particle_defs[part_def_id];
			auto &ssinst = ps.subsystems[ssid];

			float2 inv_tex_tile = vinv(float2(pdef.texture_tiles));
			for(auto &pinst : ssinst.particles) {
				float ptime = pinst.particleTime();

				float2 pos = pinst.pos + ps.pos;
				float2 size(pdef.size.sample(ptime) * 0.5f);
				float alpha = pdef.alpha.sample(ptime);

				FRect tex_rect(float2(1));
				if(pdef.texture_tiles != int2(1, 1))
					tex_rect = (tex_rect + float2(pinst.tex_tile)) * inv_tex_tile;

				auto corners = FRect(pos - size, pos + size).corners();
				FColor color(pdef.color.sample(ptime), alpha);
				for(auto &corner : corners)
					corner = rotateVector(corner - pos, pinst.rot) + pos;
				out.emplace_back(corners, tex_rect.corners(), color, part_def_id);
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
	PASSERT(valid(id));
	return m_systems[id];
}

const ParticleSystem &FXManager::get(ParticleSystemId id) const {
	PASSERT(valid(id));
	return m_systems[id];
}

ParticleSystemId FXManager::addSystem(ParticleSystemDefId def_id, float2 pos) {
	auto &def = (*this)[def_id];

	for(int n = 0; n < m_systems.size(); n++)
		if(m_systems[n].is_dead) {
			if(m_systems[n].spawn_time == m_spawn_clock)
				m_spawn_clock++;

			m_systems[n] = {pos, def_id, m_spawn_clock, def.subsystems.size()};
			initialize(m_systems[n]);
			return ParticleSystemId(n, m_spawn_clock);
		}

	m_systems.emplace_back(pos, def_id, m_spawn_clock, def.subsystems.size());
	initialize(m_systems.back());
	return ParticleSystemId(m_systems.size() - 1, m_spawn_clock);
}

void FXManager::initialize(ParticleSystem &ps) {
	for(auto &ss : ps.subsystems)
		ss.random_seed = m_random();
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
	m_particle_defs.emplace_back(move(def));
	return ParticleDefId(m_particle_defs.size() - 1);
}

EmitterDefId FXManager::addDef(EmitterDef def) {
	m_emitter_defs.emplace_back(move(def));
	return EmitterDefId(m_emitter_defs.size() - 1);
}

ParticleSystemDefId FXManager::addDef(ParticleSystemDef def) {
	m_system_defs.emplace_back(move(def));
	return ParticleSystemDefId(m_system_defs.size() - 1);
}