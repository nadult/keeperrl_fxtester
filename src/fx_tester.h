#pragma once

#include "fx_tester_base.h"
#include <fwk/gfx_base.h>
#include <fwk/sys/immutable_ptr.h>
#include <fwk/sys/unique_ptr.h>

class FXManager;
struct Spawner;

namespace fx_tester {

DEFINE_ENUM(FXTesterMode, spawn, occlusion);

class FXTester {
  public:
	using Mode = FXTesterMode;

	FXTester();

	void spawnToolMenu();
	void spawnToolInput(CSpan<InputEvent>);
	void addSpawner(int2 pos);
	void removeSpawner(int2 pos);
	void updateSpawners();

	void loadOccluders();
	void occlusionToolMenu();
	void occlusionToolInput(CSpan<InputEvent>);
	void drawOccluders(Renderer2D &) const;

	void doMenu();

	void tick(GfxDevice &device, double time_diff);
	void render() const;

	bool mainLoop(GfxDevice &device);
	static bool mainLoop(GfxDevice &device, void *this_ptr);

  private:
	static PTexture loadTexture(string file_name);
	void loadBackgrounds();

	int m_menu_width;
	int2 m_menu_size;

	fwk::IRect m_viewport;
	float2 m_view_pos; // in tiles
	int2 m_selected_tile;
	float m_zoom = 2.0f;
	float m_animation_speed = 1.0f;
	bool m_show_cursor = false;

	struct SpawnTool;
	struct OcclusionTool;

	Mode m_mode = Mode::spawn;
	UniquePtr<SpawnTool> m_spawn_tool;
	UniquePtr<OcclusionTool> m_occlusion_tool;
	UniquePtr<ImGuiWrapper> m_imgui;

	UniquePtr<FXManager> m_ps;
	fwk::vector<Spawner> m_spawners;

	fwk::vector<PTexture> m_particle_textures;
	fwk::vector<SimpleMaterial> m_particle_materials;
	PTexture m_marker_tex;

	struct Background {
		PTexture texture;
		int tile_size;
		string name;
	};

	fwk::vector<Background> m_backgrounds;
	Maybe<int> m_background_id;
};
}
