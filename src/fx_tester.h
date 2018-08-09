#pragma once

#define FWK_VEC2_CONVERSIONS                                                                                           \
  vec2(const fwk::vec2<T> &rhs) : x(rhs.x), y(rhs.y) {}                                                                \
  operator fwk::vec2<T>() const { return {x, y}; }

#define FWK_VEC3_CONVERSIONS                                                                                           \
  vec3(const fwk::vec3<T> &rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}                                                      \
  operator fwk::vec3<T>() const { return {x, y, z}; }

#include <fwk/gfx/color.h>
#include <fwk/gfx_base.h>
#include <fwk/math/box.h>
#include <fwk/sys/immutable_ptr.h>
#include <fwk/sys/unique_ptr.h>
#include <fwk_vector.h>

#undef CHECK

#include "keeperrl/fx_base.h"

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

// TODO: it would be best to drop dependency on libfwk, but we would have to:
// - rewrite imgui_wrapper (use parts of code from SDL2 imgui example)
// - rewrite input parts of FXTester
class FXTester {
  public:
  using Mode = FXTesterMode;

  FXTester(float zoom, Maybe<int> fixedFps = none);

  bool spawnEffect(string, int2, int2);
  void focusOn(int2);
  bool setBackground(string);

  void spawnToolMenu();
  void spawnToolInput(CSpan<InputEvent>);

  void loadOccluders();
  void removeOccluder(int2);
  void occlusionToolMenu();
  void occlusionToolInput(CSpan<InputEvent>);
  void drawOccluders(Renderer2D &) const;

  void doMenu();

  void tick(GfxDevice &device, double timeDiff);
  void render() const;
  void renderParticles() const;

  bool mainLoop(GfxDevice &device);
  static bool mainLoop(GfxDevice &device, void *this_ptr);

private:
  void drawCursor(Renderer2D &, int2, FColor) const;
  static PTexture loadTexture(string file_name);
  void loadBackgrounds();
  void setZoom(float2 screenPos, float zoom);

  float2 screenToTile(float2) const;
  float2 tileToScreen(int2) const;
  float2 tileToScreen(float2) const;

  int m_menuWidth;
  int2 m_menuSize;

  IRect m_viewport;
  float2 m_topLeftTile;

  int2 m_selectedTile;
  float m_zoom = 2.0f;
  float m_animationSpeed = 1.0f;
  bool m_showCursor = false;

  struct SpawnTool;
  struct OcclusionTool;

  Mode m_mode = Mode::spawn;
  UniquePtr<SpawnTool> m_spawnTool;
  UniquePtr<OcclusionTool> m_occlusionTool;
  UniquePtr<ImGuiWrapper> m_imgui;
  UniquePtr<FXManager> m_manager;
  UniquePtr<FXRenderer> m_renderer;
  PTexture m_cursor_tex;

  struct Background {
    PTexture texture;
    int tile_size;
    string name;
  };

  vector<Background> m_backgrounds;
  Maybe<int> m_backgroundId;
  Maybe<int> m_fixedFps;
};
}
