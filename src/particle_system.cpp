#include "particle_system.h"

#include <fwk/math/random.h>

using PSInstance = ParticleSystemInstance;
using PInstance = ParticleInstance;

PSInstance::ParticleSystemInstance(float2 pos, int system_id) : pos(pos), system_id(system_id) {}

void PSInstance::clear() {
	subsystems.clear();
	is_dead = true;
}

int ParticleManager::addInstance(int system_id, float2 pos) {
	// TODO: keep list of dead system instances
	for(int n = 0; n < m_instances.size(); n++)
		if(m_instances[n].is_dead) {
			m_instances[n] = {pos, system_id};
			initialize(m_instances[n]);
			return n;
		}
	m_instances.emplace_back(pos, system_id);
	initialize(m_instances.back());
	return m_instances.size() - 1;
}

void ParticleManager::initialize(PSInstance &sinst) {
	auto &sdef = m_system_defs[sinst.system_id];
	sinst.subsystems.resize(sdef.subsystems.size());
	// TODO: initial particles
}

void ParticleManager::simulate(PSInstance &sinst, float time_delta) {
	auto &sdef = m_system_defs[sinst.system_id];

	// TODO: more intuitive naming
	float anim_delta = time_delta / sdef.anim_length;

	// Animating live particles
	for(int ssid = 0; ssid < sdef.subsystems.size(); ssid++) {
		auto &pdef = m_particle_defs[sdef.subsystems[ssid].particle_id];
		auto &ssinst = sinst.subsystems[ssid];

		for(auto &pinst : ssinst.particles) {
			float ptime = pinst.particleTime();
			pinst.pos += pinst.movement * time_delta;
			pinst.rot += pinst.rot_speed * time_delta;
			pinst.life += time_delta;
		}
	}

	// Removing dead particles
	for(auto &ssinst : sinst.subsystems)
		for(int n = 0; n < ssinst.particles.size(); n++) {
			auto &pinst = ssinst.particles[n];
			if(pinst.life > pinst.max_life) {
				pinst = ssinst.particles.back();
				ssinst.particles.pop_back();
				n--;
			}
		}

	// Emitting new particles
	for(int ssid = 0; ssid < sdef.subsystems.size(); ssid++) {
		auto &pdef = m_particle_defs[sdef.subsystems[ssid].particle_id];
		auto &edef = m_emitter_defs[sdef.subsystems[ssid].emitter_id];
		auto &ssinst = sinst.subsystems[ssid];

		// TODO: random z keepera
		Random rand(ssinst.random_seed);

		float max_life = pdef.life.sample(anim_delta);
		float norm_anim_pos = sinst.anim_time / sdef.anim_length;
		float freq = edef.frequency.sample(norm_anim_pos);
		float emission = freq * time_delta + ssinst.emission_fract;
		int num_particles = int(emission);
		ssinst.emission_fract = emission - float(num_particles);

		float angle = edef.direction.sample(norm_anim_pos);
		float angle_spread = edef.direction_spread.sample(norm_anim_pos);

		float strength = edef.strength.sample(norm_anim_pos);

		for(int n = 0; n < num_particles; n++) {
			ParticleInstance new_inst;
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
}

void ParticleManager::simulate(float delta) {
	for(auto &inst : m_instances)
		if(!inst.is_dead)
			simulate(inst, delta);
	// TODO: remove dead instances
}

vector<RenderQuad> ParticleManager::genQuads() const {
	vector<RenderQuad> out;

	for(auto &sinst : m_instances) {
		if(sinst.is_dead)
			continue;
		auto &sdef = m_system_defs[sinst.system_id];

		for(int ssid = 0; ssid < sdef.subsystems.size(); ssid++) {
			int part_def_id = sdef.subsystems[ssid].particle_id;
			auto &pdef = m_particle_defs[part_def_id];
			auto &ssinst = sinst.subsystems[ssid];
			auto tex_coords = FRect(float2(1)).corners();

			for(auto &pinst : ssinst.particles) {
				float ptime = pinst.particleTime();

				float2 pos = pinst.pos + sinst.pos;
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

void ParticleManager::remove(int instance_id) {
	if(instance_id >= 0 && instance_id < m_instances.size())
		m_instances[instance_id].clear();
}
