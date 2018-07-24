#include "fcolor.h"
#include "fx_manager.h"

using SubSystemDef = ParticleSystemDef::SubSystem;

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

	pdef.color = {{{1.0f, 1.0f, 0.0f}, {0.5f, 1.0f, 0.5f}, {0.2f, 0.5f, 1.0f}}, InterpType::linear};
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

	FColor brown(Color(120, 87, 46));
	// Kiedy cząsteczki opadną pod drzewo, robią się w zasięgu cienia
	// TODO: lepiej rysować je po prostu pod cieniem
	pdef.color = {{0.0f, 0.04f, 0.06}, {brown.rgb(), brown.rgb(), brown.rgb() * 0.6f}};
	pdef.texture_name = "flakes_4x4_borders.png";
	pdef.texture_tiles = {4, 4};

	SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef));
	ssdef.max_total_particles = 4;

	ParticleSystemDef psdef;
	psdef.subsystems = {ssdef};
	psdef.anim_length = 5.0f;
	psdef.name = "splinter";
	mgr.addDef(psdef);

	// Kierunkowo to wygląda słabo, może lepiej genereować we wszystkich kierunkach
	// i najlepiej, jakby drzazgi lądowały pod drzewem!
}

static void addExplosionEffect(FXManager &mgr) {
	// TODO: tutaj trzeba zrobić tak, żeby cząsteczki które spawnują się później
	// zaczynały z innym kolorem
	EmitterDef edef;
	edef.strength_min = edef.strength_max = 15.0f;
	edef.direction = 0.0f;
	edef.direction_spread = fconstant::pi;
	edef.frequency = 60.0f;

	ParticleDef pdef;
	pdef.life = 0.5f;
	pdef.size = {{5.0f, 30.0f}};
	pdef.alpha = {{0.0f, 0.5f, 1.0f}, {0.3, 0.4, 0.0}};

	Color start_color(255, 244, 88), end_color(225, 92, 19);
	pdef.color = {{FColor(start_color).rgb(), FColor(end_color).rgb()}};
	pdef.texture_name = "clouds_soft_borders_4x4.png";
	pdef.texture_tiles = {4, 4};

	SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef));
	ssdef.max_total_particles = 20;

	ParticleSystemDef psdef;
	psdef.subsystems = {ssdef};
	psdef.anim_length = 2.0f;
	psdef.name = "explosion";
	mgr.addDef(psdef);
}

static void addRippleEffect(FXManager &mgr) {
	EmitterDef edef;

	// Czy chcemy móc definiować PSy całkowicie z pliku?

	// Jak przekazywać parametry do animacji?
	// - mogą kontrolować pozycję na jakiejś krzywej, która poźniej jest używana do czegoś tam
	// - mogą być argumentem jakiejś prostej operacji (+,-,*) wykonywanej na jakiejś pośredniej wartości
	//   (ale na końcowej / początkowej w sumie też?)
	//
	// - parametry są trzymane per system;
	//   a może różne systemy udos
	//
	// - Przy definiowaniu efektów możemy podpinać jakoś parametry ?
	//
	// - możemy zrobić standardowe shadery:
	//   - funkcja zwracająca ilość cząsteczek w danej klatce
	//   - funkcja generująca nowe cząsteczki
	//   - funkcja animująca już żyjące cząsteczki
	//
	// Parametr może być aplikowany globalnie, do subsystemu, zamiast krzywej

	// Ta animacja nie ma sprecyzowanej długości;
	// Zamiast tego może być włączona / wyłączona albo może się zwięszyć/zmniejszyć jej siła
	// krzywe które zależą od czasu animacji tracą sens;
	// animacja może być po prostu zapętlona
	edef.frequency = 1.5f;
	edef.initial_spawn_count = 1.0f;

	ParticleDef pdef;
	pdef.life = 1.5f;
	pdef.size = {{10.0f, 50.0f}};
	pdef.alpha = {{0.0f, 0.3f, 0.6f, 1.0f}, {0.0f, 0.3f, 0.5f, 0.0f}};

	pdef.color = FVec3(1.0f, 1.0f, 1.0f);
	pdef.texture_name = "torus.png";

	SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef));
	ssdef.max_active_particles = 4;

	ParticleSystemDef psdef;
	psdef.subsystems = {ssdef};
	psdef.anim_length = 1.0f;
	psdef.is_looped = true; // TODO: to nie jest brane pod uwagę na razie
	psdef.name = "ripple";
	mgr.addDef(psdef);
}

void FXManager::addDefaultDefs() {
	addTestEffect(*this);
	addSplinterEffect(*this);
	addExplosionEffect(*this);
	addRippleEffect(*this);
};
