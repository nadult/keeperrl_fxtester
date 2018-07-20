#include "fx_manager.h"

static void addTestEffect(FXManager &mgr) {
	EmitterDef edef;
	edef.strength_min = edef.strength_max = 30.0f;
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
	edef.strength_min = 20.0f;
	edef.strength_max = 60.0f;
	edef.direction = 0.0f;
	edef.direction_spread = fconstant::pi;
	edef.rotation_speed_min = -0.5f;
	edef.rotation_speed_max = 0.5f;
	edef.frequency = 999.0f;

	ParticleDef pdef;
	pdef.life = 5.0f; // life min-max ?
	pdef.size = 8.0f;
	pdef.slowdown = {{0.0f, 0.1f}, {5.0f, 1000.0f}};
	pdef.alpha = {{0.0f, 0.8f, 1.0f}, {1.0, 1.0, 0.0}};
	pdef.attract_bottom = 1.0f;

	FColor brown(IColor(120, 87, 46));
	// Kiedy cząsteczki opadną pod drzewo, robią się w zasięgu cienia
	// TODO: lepiej rysować je po prostu pod cieniem
	pdef.color = {{0.0f, 0.04f, 0.06}, {brown.rgb(), brown.rgb(), brown.rgb() * 0.6f}};
	pdef.texture_name = "flakes_4x4_borders.png";
	pdef.texture_tiles = int2(4, 4);

	ParticleSystemDef psdef;
	psdef.subsystems.emplace_back(mgr.addDef(pdef), mgr.addDef(edef));
	psdef.subsystems.back().max_total_particles = 4;
	psdef.anim_length = 5.0f;
	psdef.name = "splinter";
	mgr.addDef(psdef);

	// Kierunkowo to wygląda słabo, może lepiej genereować we wszystkich kierunkach
	// i najlepiej, jakby drzazgi lądowały pod drzewem!
}

void FXManager::addDefaultDefs() {
	addTestEffect(*this);
	addSplinterEffect(*this);
};
