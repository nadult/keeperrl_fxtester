#pragma once

#include "base.h"

#include "imgui/imgui.h"
#include "imgui_wrapper.h"
#include <fwk/filesystem.h>
#include <fwk/gfx/font_factory.h>
#include <fwk/sys/immutable_ptr.h>

#include "fx_manager.h"
#include "spawner.h"

class FXTester {
  public:
	static constexpr int tile_size = default_tile_size;

	FXTester();

	void doSpawnToolMenu();
	void doMenu();
	void addSpawner(int2 pos);
	void removeSpawner(int2 pos);
	void updateSpawners();

	void tick(GfxDevice &device, double time_diff);
	void render() const;

	bool mainLoop(GfxDevice &device);

	static bool mainLoop(GfxDevice &device, void *this_ptr);

	static PTexture loadTexture(string file_name);
	void loadBackgrounds();

	struct Background {
		PTexture texture;
		int tile_size;
		string name;
	};

  private:
	ImGuiWrapper m_imgui;

	int m_menu_width;
	int2 m_menu_size;

	IRect m_viewport;
	float2 m_view_pos; // in tiles
	int2 m_selection;
	float m_zoom = 2.0f;
	float m_animation_speed = 1.0f;

	struct SpawnTool {
		SpawnerType type = SpawnerType::single;
		ParticleSystemDefId system_id = ParticleSystemDefId(0);
	} m_spawn_tool;

	FXManager m_ps;
	vector<Spawner> m_spawners;

	vector<PTexture> m_particle_textures;
	vector<SimpleMaterial> m_particle_materials;
	PTexture m_marker_tex;

	vector<Background> m_backgrounds;
	Maybe<int> m_background_id;

	mutable float2 m_max_extents;
	FilePath m_data_path;
	Font m_font;
	bool m_exit_please = false;
};
