#include "fx_manager.h"

using InterpType = InterpolationType;

void FXManager::addDefaultDefs() {
	EmitterDef edef;
	edef.strength = 30.0f;
	edef.direction = 1.0f;
	edef.direction_spread = fconstant::pi;
	edef.frequency = {{10.0f, 55.0f, 0.0f, 0.0}, InterpType::cosine};

	edef.name = "test emitter 01";

	ParticleDef pdef;
	pdef.life = 1.0f;
	pdef.size = 32.0f;
	pdef.alpha = {{0.0f, 0.1, 0.8f, 1.0f}, {0.0, 1.0, 1.0, 0.0}, InterpType::linear};

	pdef.color = {{float3(1.0f, 1.0f, 0.0f), float3(0.5f, 1.0f, 0.5f), float3(0.2f, 0.5f, 1.0f)},
				  InterpType::linear};
	pdef.name = "test particle 01";
	pdef.texture_name = "circular.png";

	m_particle_defs.emplace_back(pdef);
	m_emitter_defs.emplace_back(edef);

	ParticleSystemDef psdef;
	psdef.subsystems.emplace_back(ParticleDefId(0), EmitterDefId(0));
	psdef.anim_length = 10.0f;
	m_system_defs.emplace_back(psdef);
};
