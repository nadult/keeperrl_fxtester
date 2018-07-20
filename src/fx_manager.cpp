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

		for(auto &pinst : ssinst.particles) {
			float ptime = pinst.particleTime();
			pinst.pos += pinst.movement * time_delta;
			pinst.rot += pinst.rot_speed * time_delta;
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
		auto &pdef = m_particle_defs[def.subsystems[ssid].particle_id];
		auto &edef = m_emitter_defs[def.subsystems[ssid].emitter_id];
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

		float strength = edef.strength.sample(norm_anim_pos);

		for(int n = 0; n < num_particles; n++) {
			Particle new_inst;
			new_inst.pos = float2();
			float pangle;
			if(angle_spread < fconstant::pi)
				pangle = angle + rand.uniform(-angle_spread, angle_spread);
			else
				pangle = rand.uniform(0.0f, fconstant::pi * 2.0f);
			float2 pdir = angleToVector(pangle);
			new_inst.movement = pdir * strength;
			new_inst.rot = 0.0f;
			new_inst.rot_speed = 0.0f;
			new_inst.life = 0.0f;
			new_inst.max_life = max_life;
			ssinst.particles.emplace_back(new_inst);
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
			auto tex_coords = FRect(float2(1)).corners();

			for(auto &pinst : ssinst.particles) {
				float ptime = pinst.particleTime();

				float2 pos = pinst.pos + ps.pos;
				float2 size(pdef.size.sample(ptime) * 0.5f);
				float alpha = pdef.alpha.sample(ptime);

				auto corners = FRect(pos - size, pos + size).corners();
				FColor color(pdef.color.sample(ptime), alpha);
				// TODO: rotate
				out.emplace_back(corners, tex_coords, color, part_def_id);
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
	auto &def = (*this)[ps.def_id];
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
