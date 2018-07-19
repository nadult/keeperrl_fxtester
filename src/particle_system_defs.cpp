#include "particle_system.h"

using InterpType = InterpolationType;

void ParticleManager::addDefaults() {
	EmitterDef edef;
	edef.strength = 5.0f;
	edef.direction = 1.0f;
	edef.direction_spread = fconstant::pi;
	edef.frequency = {{10.0f, 100.0f}, InterpType::cosine};
	edef.name = "test emitter 01";

	ParticleDef pdef;
	pdef.life = 15.0f;
	pdef.size = 32.0f;
	pdef.alpha = {{0.0f, 0.1, 0.9f, 1.0f}, {0.0, 1.0, 1.0, 0.1}, InterpType::linear};
	pdef.color = {{float3(1.0f, 1.0f, 0.0f), float3(0.5f, 1.0f, 0.5f), float3(0.2f, 0.5f, 1.0f)},
				  InterpType::linear};
	pdef.name = "test particle 01";
	pdef.texture_name = "circular.png";

	m_particle_defs.emplace_back(pdef);
	m_emitter_defs.emplace_back(edef);

	ParticleSystemDef psdef;
	psdef.subsystems.emplace_back(0, 0);
	psdef.anim_length = 10.0f;
	m_system_defs.emplace_back(psdef);
};
