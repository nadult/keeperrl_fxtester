#pragma once

#include "fx_tester_base.h"

#include "imgui/imgui.h"
#include "imgui_wrapper.h"
#include <fwk/filesystem.h>
#include <fwk/math/box.h>
#include <fwk/sys/immutable_ptr.h>
#include <fwk/sys/unique_ptr.h>

#include "fx_manager.h"
#include "spawner.h"

DEFINE_ENUM(FXTesterMode, spawn, occlusion);

class FXTester {
  public:
	using Mode = FXTesterMode;
	static constexpr int tile_size = default_tile_size;

	FXTester();

	void spawnToolMenu();
	void spawnToolInput(fwk::CSpan<fwk::InputEvent>);
	void addSpawner(fwk::int2 pos);
	void removeSpawner(fwk::int2 pos);
	void updateSpawners();

	void loadOccluders();
	void occlusionToolMenu();
	void occlusionToolInput(fwk::CSpan<fwk::InputEvent>);
	void drawOccluders(fwk::Renderer2D &) const;

	void doMenu();

	void tick(fwk::GfxDevice &device, double time_diff);
	void render() const;

	bool mainLoop(fwk::GfxDevice &device);
	static bool mainLoop(fwk::GfxDevice &device, void *this_ptr);

  private:
	static fwk::PTexture loadTexture(string file_name);
	void loadBackgrounds();

	ImGuiWrapper m_imgui;
	int m_menu_width;
	fwk::int2 m_menu_size;

	fwk::IRect m_viewport;
	fwk::float2 m_view_pos; // in tiles
	fwk::int2 m_selected_tile;
	float m_zoom = 2.0f;
	float m_animation_speed = 1.0f;
	bool m_show_cursor = false;

	struct SpawnTool;
	struct OcclusionTool;

	Mode m_mode = Mode::spawn;
	fwk::UniquePtr<SpawnTool> m_spawn_tool;
	fwk::UniquePtr<OcclusionTool> m_occlusion_tool;

	FXManager m_ps;
	fwk::vector<Spawner> m_spawners;

	fwk::vector<fwk::PTexture> m_particle_textures;
	fwk::vector<fwk::SimpleMaterial> m_particle_materials;
	fwk::PTexture m_marker_tex;

	struct Background {
		fwk::PTexture texture;
		int tile_size;
		string name;
	};

	fwk::vector<Background> m_backgrounds;
	fwk::Maybe<int> m_background_id;
};
