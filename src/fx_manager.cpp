#include "fx_manager.h"

#include "fcolor.h"
#include "fmath.h"
#include "frect.h"

FXManager::FXManager() { addDefaultDefs(); }
FXManager::~FXManager() = default;

SubSystemContext FXManager::ssctx(ParticleSystem &ps, int ssid) {
	auto &ss = ps[ssid];
	const auto &psdef = (*this)[ps.def_id];
	const auto &ssdef = psdef[ssid];
	const auto &pdef = (*this)[psdef[ssid].particle_id];
	const auto &edef = (*this)[psdef[ssid].emitter_id];
	return {ps, ss, psdef, ssdef, pdef, edef, ssid};
}

void FXManager::simulate(ParticleSystem &ps, float time_delta) {
	auto &psdef = (*this)[ps.def_id];

	float norm_anim_time = ps.anim_time / psdef.anim_length;
	float inv_time_delta = 1.0f / time_delta;

	// Animating live particles
	for(int ssid = 0; ssid < (int)psdef.subsystems.size(); ssid++) {
		auto &ss = ps[ssid];
		auto &ssdef = psdef[ssid];

		AnimationContext ctx{ssctx(ps, ssid), {},		  ps.anim_time,
							 norm_anim_time,  time_delta, inv_time_delta};
		ctx.rand.init(ss.random_seed);

		for(auto &pinst : ss.particles)
			ssdef.animate_func(ctx, pinst);

		ss.random_seed = ctx.randomSeed();
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
	for(int ssid = 0; ssid < (int)psdef.subsystems.size(); ssid++) {
		auto &ss = ps[ssid];
		const auto &ssdef = psdef[ssid];

		AnimationContext ctx{ssctx(ps, ssid), {},		  ps.anim_time,
							 norm_anim_time,  time_delta, inv_time_delta};
		ctx.rand.init(ss.random_seed);

		EmissionState em;
		float emission = ssdef.prepare_func(ctx, em) + ss.emission_fract;
		int num_particles = int(emission);
		ss.emission_fract = emission - float(num_particles);

		int max_particles = min(ssdef.max_active_particles - (int)ss.particles.size(),
								ssdef.max_total_particles - ss.total_particles);
		num_particles = min(num_particles, max_particles);
		ss.total_particles += num_particles;

		for(int n = 0; n < num_particles; n++) {
			Particle new_inst;
			ssdef.emit_func(ctx, em, new_inst);
			ss.particles.emplace_back(new_inst);
		}
		ss.random_seed = ctx.randomSeed(); // TODO: save random state properly?
	}

	ps.anim_time += time_delta;
	if(ps.anim_time > psdef.anim_length) {
		if(psdef.is_looped)
			ps.anim_time -= psdef.anim_length;
		else
			ps.kill();
	}
}

void FXManager::simulate(float delta) {
	for(auto &inst : m_systems)
		if(!inst.is_dead)
			simulate(inst, delta);
}

std::vector<DrawParticle> FXManager::genQuads() {
	std::vector<DrawParticle> out;
	// TODO(opt): reserve

	for(auto &ps : m_systems) {
		if(ps.is_dead)
			continue;
		auto &psdef = (*this)[ps.def_id];

		for(int ssid = 0; ssid < (int)psdef.subsystems.size(); ssid++) {
			auto &ss = ps[ssid];
			auto &ssdef = psdef[ssid];
			auto &pdef = m_particle_defs[ssdef.particle_id];
			DrawContext ctx{ssctx(ps, ssid), vinv(FVec2(pdef.texture_tiles))};

			for(auto &pinst : ss.particles) {
				DrawParticle dparticle;
				ctx.ssdef.draw_func(ctx, pinst, dparticle);
				dparticle.particle_def_id = ssdef.particle_id;
				out.emplace_back(dparticle);
			}
		}
	}

	return out;
}

bool FXManager::valid(ParticleSystemId id) const {
	return id >= 0 && id < (int)m_systems.size() && m_systems[id].spawn_time == id.spawnTime();
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

	for(int n = 0; n < (int)m_systems.size(); n++)
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

std::vector<ParticleSystemId> FXManager::aliveSystems() const {
	std::vector<ParticleSystemId> out;
	out.reserve(m_systems.size());

	for(int n = 0; n < (int)m_systems.size(); n++)
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
