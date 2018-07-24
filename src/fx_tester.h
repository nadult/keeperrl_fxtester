#pragma once

#define FWK_VEC2_CONVERSIONS                                                                       \
	vec2(const fwk::vec2<T> &rhs) : x(rhs.x), y(rhs.y) {}                                          \
	operator fwk::vec2<T>() const { return {x, y}; }

#define FWK_VEC3_CONVERSIONS                                                                       \
	vec3(const fwk::vec3<T> &rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}                                \
	operator fwk::vec3<T>() const { return {x, y, z}; }

#include <fwk/gfx/color.h>
#include <fwk/gfx_base.h>
#include <fwk/math/box.h>
#include <fwk/sys/immutable_ptr.h>
#include <fwk/sys/unique_ptr.h>
#include <fwk_vector.h>

#undef CHECK

#include "fx_base.h"

namespace fx::tester {

class ImGuiWrapper;
using FRect = fwk::FRect;
using IRect = fwk::IRect;
using FColor = fwk::FColor;
using IColor = fwk::IColor;
template <class T> using vector = fwk::vector<T>;
using fwk::clamp;

using namespace fwk;

DEFINE_ENUM(FXTesterMode, spawn, occlusion);

class FXTester {
  public:
	using Mode = FXTesterMode;

	FXTester();

	void spawnToolMenu();
	void spawnToolInput(CSpan<InputEvent>);

	void loadOccluders();
	void removeOccluder(int2);
	void occlusionToolMenu();
	void occlusionToolInput(CSpan<InputEvent>);
	void drawOccluders(Renderer2D &) const;

	void doMenu();

	void tick(GfxDevice &device, double time_diff);
	void render() const;

	bool mainLoop(GfxDevice &device);
	static bool mainLoop(GfxDevice &device, void *this_ptr);

  private:
	void drawCursor(Renderer2D &, int2, FColor) const;
	static PTexture loadTexture(string file_name);
	void loadBackgrounds();
	void setZoom(float2 screen_pos, float zoom);

	float2 screenToTile(float2) const;
	float2 tileToScreen(int2) const;
	float2 tileToScreen(float2) const;

	int m_menu_width;
	int2 m_menu_size;

	IRect m_viewport;
	float2 m_top_left_tile;

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

	vector<PTexture> m_particle_textures;
	vector<SimpleMaterial> m_particle_materials;
	PTexture m_cursor_tex;

	struct Background {
		PTexture texture;
		int tile_size;
		string name;
	};

	vector<Background> m_backgrounds;
	Maybe<int> m_background_id;
};
}
