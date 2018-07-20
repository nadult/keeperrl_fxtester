#include "fx_manager.h"

using InterpType = InterpolationType;

static void addTestEffect(FXManager &mgr) {
	EmitterDef edef;
	edef.strength = 30.0f;
	edef.direction = 0.0f;
	edef.direction_spread = fconstant::pi;
	edef.frequency = {{10.0f, 55.0f, 0.0f, 0.0}, InterpType::cosine};

	ParticleDef pdef;
	pdef.life = 1.0f;
	pdef.size = 32.0f;
	pdef.alpha = {{0.0f, 0.1, 0.8f, 1.0f}, {0.0, 1.0, 1.0, 0.0}, InterpType::linear};

	pdef.color = {{float3(1.0f, 1.0f, 0.0f), float3(0.5f, 1.0f, 0.5f), float3(0.2f, 0.5f, 1.0f)},
				  InterpType::linear};
	pdef.texture_name = "circular.png";

	ParticleSystemDef psdef;
	psdef.subsystems.emplace_back(mgr.addDef(pdef), mgr.addDef(edef));
	psdef.anim_length = 10.0f;
	psdef.name = "fountain";
	mgr.addDef(psdef);
}

static void addSplinterEffect(FXManager &mgr) {
	EmitterDef edef;
	edef.strength = 150.0f;
	edef.direction = 0.0f;
	edef.direction_spread = 0.5f;
	edef.frequency = {{150.0f, 0.0f, 0.0f, 0.0}, InterpType::cosine};

	ParticleDef pdef;
	pdef.life = 0.1f;
	pdef.size = 7.0f;
	pdef.alpha = {{0.0f, 0.8f, 1.0f}, {1.0, 1.0, 0.0}, InterpType::linear};

	FColor brown(IColor(208, 116, 60));
	pdef.color = {{brown.rgb(), brown.rgb() * 0.2f}, InterpType::linear};
	// TODO: fix texture
	pdef.texture_name = "circular.png";

	ParticleSystemDef psdef;
	psdef.subsystems.emplace_back(mgr.addDef(pdef), mgr.addDef(edef));
	psdef.anim_length = 1.0f;
	psdef.name = "splinter";
	mgr.addDef(psdef);

	// Kierunkowo to wygląda słabo, może lepiej genereować we wszystkich kierunkach
	// i najlepiej, jakby drzazgi lądowały pod drzewem!
}

void FXManager::addDefaultDefs() {
	addTestEffect(*this);
	addSplinterEffect(*this);
};
