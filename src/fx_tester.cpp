#include "fx_tester.h"

#include "keeperrl/fx_manager.h"
#include "keeperrl/fx_defs.h"
#include "keeperrl/fx_renderer.h"

#include "imgui/imgui.h"
#include "imgui_funcs.h"
#include "imgui_wrapper.h"

#include <fwk/filesystem.h>
#include <fwk/gfx/dtexture.h>
#include <fwk/gfx/gfx_device.h>
#include <fwk/gfx/material.h>
#include <fwk/gfx/program_binder.h>
#include <fwk/gfx/opengl.h>
#include <fwk/gfx/renderer2d.h>
#include <fwk/math/box.h>
#include <fwk/parse.h>
#include <fwk/sys/backtrace.h>
#include <fwk/sys/input.h>
#include <fwk/sys/stream.h>

namespace SDL {
typedef unsigned int GLenum;
typedef unsigned int GLuint;
}

#include "keeperrl/renderer.h"

namespace fx::tester {

static constexpr int tile_size = Renderer::nominalSize;

// ------------------------------------------------------------------------------------------------
// ------------------------------------- SPAWN TOOL -----------------------------------------------

struct Spawner {
  void update(FXManager& manager) {
    if (isDead)
      return;

    if (manager.dead(instanceId)) {
      if (spawnCount > 0 && !autoRespawn) {
        isDead = true;
        return;
      }

      spawnCount++;
      instanceId = manager.addSystem(systemName, initConfig);
    }

    if (!manager.dead(instanceId))
      manager.get(instanceId).params = params;
  }

  IVec2 tilePos;
  ParticleSystemId instanceId;
  int spawnCount = 0;
  FXName systemName = FXName::FIRE;
  bool isDead = false;
  bool autoRespawn = false;

  SystemParams params;
  InitConfig initConfig;
};

struct FXTester::SpawnTool {
  SpawnTool() {
    defaultSpawner.initConfig.targetOffset = {1.0f, 0.0f};
  }

  void update(FXManager &mgr) {
    for (auto& spawner : spawners)
      spawner.update(mgr);

    for(int n = 0; n < spawners.size(); n++)
      if (spawners[n].isDead) {
        if(selection_id == n)
          selection_id = -1;
        if(selection_id == (int)spawners.size() - 1)
          selection_id = n;
        spawners[n] = spawners.back();
        spawners.pop_back();
        n--;
      }
    if(selection_id >= spawners.size())
      selection_id = -1;
  }

  void add(int2 pos, float2 off) {
    selection_id = spawners.size();
    auto newSpawner = defaultSpawner;
    newSpawner.tilePos = pos;
    newSpawner.initConfig.pos = (FVec2(pos.x, pos.y) + FVec2(0.5f)) * float(tile_size);
    newSpawner.initConfig.targetOffset = off * float(tile_size);
    spawners.emplace_back(newSpawner);
  }

  void add(int2 pos) {
    add(pos, defaultSpawner.initConfig.targetOffset);
  }

  void select(int2 pos) {
    selection_id = -1;
    for(int n = 0; n < spawners.size(); n++)
      if (spawners[n].tilePos == pos) {
        selection_id = n;
        break;
      }
  }

  Spawner *selection() { return selection_id == -1 ? nullptr : &spawners[selection_id]; }

  void remove(FXManager &mgr, int2 pos) {
    for(auto &spawner : spawners)
      if (spawner.tilePos == pos) {
        mgr.kill(spawner.instanceId, false);
        spawner.isDead = true;
      }
  }

  vector<Spawner> spawners;
  int selection_id = -1;
  Spawner defaultSpawner;
};

void FXTester::spawnToolMenu() {
  auto &tool = *m_spawnTool;
  auto& def = tool.defaultSpawner;

  auto names = transform(m_names, [](const auto& str) { return str.c_str(); });
  selectIndex("New system", def.systemName, names);
  ImGui::InputFloat2("targetOffset", def.initConfig.targetOffset.v);
  ImGui::Checkbox("Auto respawn", &def.autoRespawn);
  ImGui::SameLine();
  auto& snapshotKey = def.initConfig.snapshotKey;
  bool useSnapshot = !!snapshotKey;
  if (ImGui::Checkbox("Use snapshots", &useSnapshot))
    snapshotKey = useSnapshot ? SnapshotKey() : optional<SnapshotKey>();

  if (snapshotKey) {
    int id = 0;
    bool apply = false;
    for (auto& scalar : snapshotKey->scalar) {
      char name[128];
      snprintf(name, sizeof(name), "scalar #%d", id++);
      if (ImGui::SliderFloat(name, &scalar, 0.0f, 1.0f))
        apply = true;
    }
    if (apply)
      snapshotKey->apply(def.params);
    }

  ImGui::Text("LMB: add spawner\ndel: remove spawners under cursor");
  ImGui::Text("LMB + ctrl: select\n");
  ImGui::Separator();

  if(auto *sel = tool.selection()) {
    float anim_time = 0.0f;
    int num_active = 0, num_total = 0;

    if (m_manager->alive(sel->instanceId)) {
      auto &ps = m_manager->get(sel->instanceId);
      num_active = ps.numActiveParticles();
      num_total = ps.numTotalParticles();
      anim_time = ps.animTime;
    }

    ImGui::Text("Animation time: %f", anim_time);
    ImGui::Text("Active particles: %d", num_active);
    ImGui::Text("Total particles: %d\n", num_total);
    ImGui::Separator();

    ImGui::Text("Params:");
    int idx = 0;
    for(auto &f : sel->params.scalar) {
      char title[256]; // TODO: proper formatting
      snprintf(title, sizeof(title), "Scalar #%d", idx++);
      ImGui::SliderFloat(title, &f, 0.0f, 1.0f);
    }
    idx = 0;
    for(auto &c : sel->params.color) {
      char title[256]; // TODO: proper formatting
      snprintf(title, sizeof(title), "Color #%d", idx++);
      ImGui::ColorEdit3(title, c.v);
    }

    const char *dir_names[8] = {"North (0,-1)",       "South (0,+1)",       "E (+1,0)",           "W (-1,0)",
                                "North east (+1,-1)", "North west (-1,-1)", "South east (+1,+1)", "South west (-1,+1)"};
    idx = 0;
    for(auto &d : sel->params.dir) {
      char title[256]; // TODO: proper formatting
      snprintf(title, sizeof(title), "Dir #%d", idx++);
      selectIndex(title, d, dir_names);
    }
  }
}

void FXTester::spawnToolInput(CSpan<InputEvent> events) {
  auto &tool = *m_spawnTool;
  for(auto event : events) {
    if(event.keyDown(InputKey::del))
      tool.remove(*m_manager, m_selectedTile);
    if(event.mouseButtonDown(InputButton::left)) {
      if(event.mods() & InputModifier::lctrl)
        tool.select(m_selectedTile);
      else
        tool.add(m_selectedTile);
    }
  }
}

// ------------------------------------------------------------------------------------------------
// ----------------------------------- OCCLUSION TOOL ---------------------------------------------

struct FXTester::OcclusionTool {
  vector<pair<PTexture, string>> textures;
  vector<pair<int, int2>> occluders;
  int new_occluder_id = 0;
};

void FXTester::loadOccluders() {
  auto &tool = *m_occlusionTool;
  auto occ_dir = "data/occluders/", occ_suffix = ".png";
  for(auto element : findFiles(occ_dir, occ_suffix)) {
    string file_name = occ_dir + element + occ_suffix;
    tool.textures.emplace_back(loadTexture(file_name), element);
  }
}

void FXTester::removeOccluder(int2 pos) {
  auto &occluders = m_occlusionTool->occluders;
  for(int n = 0; n < occluders.size(); n++)
    if(occluders[n].second == pos) {
      occluders[n] = occluders.back();
      occluders.pop_back();
      n--;
    }
}

void FXTester::occlusionToolMenu() {
  auto &tool = *m_occlusionTool;
  auto names = transform(tool.textures, [](const auto &pair) { return pair.second.c_str(); });
  selectIndex("New occluder", tool.new_occluder_id, names);

  if(ImGui::Button("Clear occluders"))
    tool.occluders.clear();

  ImGui::Text("LMB: add occluder\ndel: remove occluder");
}

void FXTester::occlusionToolInput(CSpan<InputEvent> events) {
  auto &tool = *m_occlusionTool;

  for(auto &event : events) {
    if(event.mouseButtonDown(InputButton::left) && !tool.textures.empty())
      tool.occluders.emplace_back(tool.new_occluder_id, m_selectedTile);
    if(event.keyDown(InputKey::del))
      removeOccluder(m_selectedTile);
  }
}

void FXTester::drawOccluders(Renderer2D &out) const {
  auto &tool = *m_occlusionTool;
  auto tile_rect = FRect(tileToScreen(int2(1)));

  for(auto &occluder : tool.occluders) {
    auto tex = tool.textures[occluder.first].first;
    out.addFilledRect(tile_rect + tileToScreen(occluder.second), tex);
  }
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

FXTester::FXTester(float zoom, Maybe<int> fixedFps)
    : m_viewport(GfxDevice::instance().windowSize()), m_fixedFps(fixedFps) {
  m_imgui.emplace(GfxDevice::instance(), ImGuiStyleMode::mini);
  m_manager.emplace();
  m_renderer.emplace(DirectoryPath("data/particles/"), *m_manager);

  m_spawnTool.emplace();
  m_occlusionTool.emplace();

  m_cursorTex = loadTexture("data/cursor.png");
  loadBackgrounds();
  loadOccluders();

  //m_zoomFboChannels = 4.0f;
  //m_showFboChannels = FBOChannel::add_alpha | FBOChannel::add_rgb;

  for (auto fxName : ENUM_ALL(FXName))
    m_names.emplace_back(EnumInfo<FXName>::getString(fxName));

  setZoom(float2(m_viewport.size()) * 0.25f, zoom);
}

bool FXTester::spawnEffect(string name, int2 pos, int2 toff) {
  if (auto fxId = EnumInfo<FXName>::fromStringSafe(name)) {
    auto& def = m_spawnTool->defaultSpawner;
    auto oldRespawn = def.autoRespawn;
    def.autoRespawn = true;
    def.systemName = *fxId;
    m_spawnTool->add(pos, float2(toff));
    def.autoRespawn = oldRespawn;
    return true;
    }

  return false;
}

void FXTester::focusOn(int2 pos) { m_topLeftTile = float2(pos - int2(screenToTile(float2(m_viewport.size())) * 0.5)); }

bool FXTester::setBackground(string name) {
  if(name == "disabled") {
    m_backgroundId = fwk::none;
    return true;
  }
  for(int n = 0; n < m_backgrounds.size(); n++)
    if(m_backgrounds[n].name == name) {
      m_backgroundId = n;
      return true;
    }
  return false;
}

float2 FXTester::screenToTile(float2 spos) const { return spos / float(tile_size * m_zoom); }
float2 FXTester::tileToScreen(int2 tpos) const { return float2(tpos) * float(tile_size) * m_zoom; }
float2 FXTester::tileToScreen(float2 tpos) const { return tpos * float(tile_size) * m_zoom; }

void FXTester::setZoom(float2 screenPos, float zoom) {
  float2 oldPos = screenToTile(screenPos);
  m_zoom = clamp(zoom, 0.25f, 10.0f);
  float2 new_pos = screenToTile(screenPos);
  m_topLeftTile += oldPos - new_pos;
}

void FXTester::doMenu() {
  ImGui::Begin("FXTester", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  ImGui::SetWindowSize(m_menuSize);
  static bool initialized = false;
  if(!initialized) {
    ImGui::SetWindowPos(ImVec2(5, 5));
    initialized = true;
  }

  {
    float zoom = m_zoom;
    if(ImGui::InputFloat("Zoom", &zoom))
      setZoom(float2(m_viewport.size()) * 0.5f, clamp(zoom, 0.25f, 10.0f));
  }

  if (ImGui::InputFloat("Anim speed", &m_animationSpeed))
    m_animationSpeed = clamp(m_animationSpeed, 0.0f, 100.0f);
  ImGui::Checkbox("Show cursor", &m_showCursor);
  ImGui::Checkbox("Use FBO", &m_renderer->useFramebuffer);

  if (m_renderer->useFramebuffer) {
    ImGui::SameLine();
    if (ImGui::Button("Show FBO channels"))
      ImGui::OpenPopup("select_channels");
    if (ImGui::BeginPopup("select_channels")) {
      for (auto channel : all<FBOChannel>()) {
        bool selected = (bool)(m_showFboChannels & channel);
        if (ImGui::Checkbox(fwk::toString(channel), &selected)) {
          if (selected)
            m_showFboChannels |= channel;
          else
            m_showFboChannels &= ~channel;
        }
      }
      ImGui::PushItemWidth(100.0f);
      if (ImGui::InputFloat("zoom", &m_zoomFboChannels))
        m_zoomFboChannels = clamp(m_zoomFboChannels, 1.0f, 8.0f);
      ImGui::PopItemWidth();
      ImGui::EndPopup();
    }
  }

  if(ImGui::Button("Select background"))
    ImGui::OpenPopup("select_back");

  ImGui::Text("%s", format("Cursor: %", m_selectedTile).c_str());

  if(ImGui::BeginPopup("select_back")) {
    if (ImGui::MenuItem("disabled", nullptr, !m_backgroundId))
      m_backgroundId = fwk::none;
    for(int n = 0; n < m_backgrounds.size(); n++) {
      auto &back = m_backgrounds[n];
      if (ImGui::MenuItem(back.name.c_str(), nullptr, m_backgroundId == n)) {
        m_backgroundId = n;
      }
    }
    ImGui::EndPopup();
  }

  ImGui::Separator();

  selectEnum("Mode", m_mode);
  if(m_mode == Mode::spawn)
    spawnToolMenu();
  else
    occlusionToolMenu();

  static bool showTestWindow = 0;
  if (showTestWindow) {
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
    ImGui::ShowTestWindow(&showTestWindow);
  }

  m_menuWidth = 220;
  m_menuSize = vmax(m_menuSize, int2(m_menuWidth + 20, ImGui::GetCursorPosY()));

  ImGui::End();
}

void FXTester::tick(GfxDevice &device, double timeDiff) {
  m_manager->simulateStable(timeDiff * m_animationSpeed);

  auto events = device.inputEvents();
  m_imgui->beginFrame(device);
  doMenu();
  events = m_imgui->finishFrame(device);

  for(auto event : events) {
    if(event.keyDown(InputKey::f11)) {
      auto &gfx_device = GfxDevice::instance();
      auto flags = gfx_device.isWindowFullscreen() ? GfxDeviceFlags() : GfxDeviceOpt::fullscreen_desktop;
      gfx_device.setWindowFullscreen(flags);
    }
    if(event.keyDown(InputKey::f1))
      m_mode = Mode::spawn;
    else if(event.keyDown(InputKey::f2))
      m_mode = Mode::occlusion;

    if(event.mouseButtonPressed(InputButton::right))
      m_topLeftTile -= screenToTile(float2(event.mouseMove()));
    if(event.isMouseOverEvent()) {
      float2 sel_tile = screenToTile((float2)event.mousePos()) + m_topLeftTile;
      m_selectedTile = (int2)vfloor(sel_tile);
      if(int zoom = event.mouseWheel())
        setZoom((float2)event.mousePos(), m_zoom * (zoom > 0 ? 1.25f : 0.8f));
    }
  }

  if(m_mode == Mode::spawn)
    spawnToolInput(events);
  else if(m_mode == Mode::occlusion)
    occlusionToolInput(events);

  m_spawnTool->update(*m_manager);
}

void FXTester::drawCursor(Renderer2D &out, int2 tile_pos, FColor color) const {
  float2 sel_pos = float2(tile_pos * tile_size) - float2(2);
  auto sel_rect = FRect(sel_pos, sel_pos + float2(tile_size + 4)) * m_zoom;
  out.addFilledRect(sel_rect, SimpleMaterial(m_cursorTex, color));
}

void FXTester::renderParticles() const {
  ProgramBinder::unbind();
  SDL::glPushAttrib(GL_VIEWPORT_BIT);
  SDL::glMatrixMode(GL_PROJECTION);
  SDL::glLoadIdentity();
  SDL::glViewport(0, 0, m_viewport.width(), m_viewport.height());
  SDL::glOrtho(0.0, m_viewport.width(), m_viewport.height(), 0.0, -1.0, 1.0);
  SDL::glMatrixMode(GL_MODELVIEW);
  SDL::glLoadIdentity();

  FVec2 offset = m_topLeftTile * float(tile_size) * m_zoom;
  m_renderer->draw(m_zoom, -offset.x, -offset.y, m_viewport.width(), m_viewport.height());
  SDL::glPopAttrib();
  // Don't care about matrices here, we're not using fixed function
}

void FXTester::render() const {
  GfxDevice::clearColor(FColor(0.1, 0.1, 0.1));

  Renderer2D out(m_viewport);
  out.setViewPos(tileToScreen(m_topLeftTile));
  float tile_to_screen = m_zoom * float(tile_size);
  if (m_backgroundId) {
    auto &back = m_backgrounds[*m_backgroundId];
    auto size = float2(back.texture->size()) * tile_to_screen / float(back.tile_size);
    out.addFilledRect(FRect(size), back.texture);
  }
  out.render();

  renderParticles();

  if (m_showFboChannels)
    drawFboChannels();

  drawOccluders(out);
  if (m_showCursor)
    drawCursor(out, m_selectedTile, FColor(ColorId::white, 0.2f));
  if (m_spawnTool->selection_id != -1)
    drawCursor(out, m_spawnTool->spawners[m_spawnTool->selection_id].tilePos, FColor(ColorId::blue, 0.2f));
  out.render();
}

void FXTester::drawFboChannels() const {
  auto ids = m_renderer->fboIds();
  auto texSize = m_renderer->fboSize();

  int defaultMode = 0, defaultOp = 0, defaultSrc = 0, defaultCombine = 0;
  SDL::glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &defaultMode);
  SDL::glGetTexEnviv(GL_TEXTURE_ENV, GL_OPERAND0_RGB, &defaultOp);
  SDL::glGetTexEnviv(GL_TEXTURE_ENV, GL_SRC0_RGB, &defaultSrc);
  SDL::glGetTexEnviv(GL_TEXTURE_ENV, GL_COMBINE_RGB, &defaultCombine);

  FVec2 pos(m_viewport.width() - texSize.x - 20, 20);
  SDL::glDisable(GL_BLEND);
  glColor(Color::WHITE);

  for (auto channel : all<FBOChannel>()) {
    if (!(m_showFboChannels & channel))
      continue;

    auto id = ::isOneOf(channel, FBOChannel::blend_alpha, FBOChannel::blend_rgb) ? ids.first : ids.second;
    bool rgbMode = ::isOneOf(channel, FBOChannel::blend_rgb, FBOChannel::add_rgb);

    SDL::glDisable(GL_TEXTURE_2D);
    FVec2 endPos = pos + FVec2(texSize);
    glQuad(pos.x - 2.0f, pos.y - 2.0f, endPos.x + 2.0f, endPos.y + 2.0f);

    SDL::glEnable(GL_TEXTURE_2D);
    SDL::glBindTexture(GL_TEXTURE_2D, id);
    if (rgbMode) {
      SDL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
      SDL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
      SDL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
      SDL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    } else {
      SDL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
      SDL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
      SDL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
      SDL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_ALPHA);
    }

    FVec2 texRange = FVec2(1.0f / m_zoomFboChannels);
    FVec2 minTex = FVec2(0.5) - texRange * 0.5f;
    FVec2 maxTex = FVec2(0.5) + texRange * 0.5f;

    SDL::glBegin(GL_QUADS);
    SDL::glTexCoord2f(minTex.x, minTex.y), SDL::glVertex2f(pos.x, endPos.y);
    SDL::glTexCoord2f(maxTex.x, minTex.y), SDL::glVertex2f(endPos.x, endPos.y);
    SDL::glTexCoord2f(maxTex.x, maxTex.y), SDL::glVertex2f(endPos.x, pos.y);
    SDL::glTexCoord2f(minTex.x, maxTex.y), SDL::glVertex2f(pos.x, pos.y);
    SDL::glEnd();
    pos += FVec2(0.0f, texSize.y + 20);
  }

  SDL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, defaultMode);
  SDL::glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, defaultCombine);
  SDL::glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, defaultSrc);
  SDL::glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, defaultOp);
  SDL::glEnable(GL_BLEND);
}

bool FXTester::mainLoop(GfxDevice &device) {
  double time = getTime();
  static double lastTime = time - 1.0 / 60.0;
  double timeDiff = time - lastTime;
  lastTime = time;

  if (m_fixedFps) {
    double desiredFrameTime = 1.0 / double(*m_fixedFps);
    if (timeDiff < desiredFrameTime) {
      fwk::sleep(desiredFrameTime - timeDiff);
      lastTime = getTime();
    }
    timeDiff = desiredFrameTime;
  } else {
    timeDiff = clamp(timeDiff, 1 / 240.0, 1 / 5.0);
  }

  m_viewport = IRect(device.windowSize());

  tick(device, timeDiff);
  render();
  m_imgui->drawFrame(GfxDevice::instance());

  return true;
}

void FXTester::loadBackgrounds() {
  auto bkg_dir = "data/backgrounds/", bkg_suffix = ".png";
  for(auto element : findFiles(bkg_dir, bkg_suffix)) {
    string file_name = bkg_dir + element + bkg_suffix;
    int tile_size = 48;
    if(element.find("_24") == element.size() - 3)
      tile_size = 24;
    m_backgrounds.emplace_back(loadTexture(file_name), tile_size, element);
  }
  if(!m_backgrounds.empty())
    m_backgroundId = 0;
}

bool FXTester::mainLoop(GfxDevice &device, void *this_ptr) { return ((FXTester *)this_ptr)->mainLoop(device); }

PTexture FXTester::loadTexture(string file_name) {
  Loader loader(file_name);
  return make_immutable<DTexture>(file_name, loader);
}

extern "C" {
int main(int argc, char **argv) {
  double time = getTime();
  int2 resolution(1300, 800);
  GfxDeviceFlags gfx_flags = GfxDeviceOpt::resizable | GfxDeviceOpt::vsync | GfxDeviceOpt::opengl_debug_handler;
  Backtrace::t_default_mode = BacktraceMode::full;

  FatalLog.addOutput(DebugOutput::crash());
  FatalLog.addOutput(DebugOutput::toStream(std::cerr));
  InfoLog.addOutput(DebugOutput::toStream(std::cerr));

  string background;
  struct SpawnCommand {
    string name;
    int2 pos, off;
  };
  vector<SpawnCommand> spawns;

  float zoom = 2.0f;
  Maybe<int> fixedFps;

  for(int n = 1; n < argc; n++) {
    string argument = argv[n];
    if(argument == "-res") {
      ASSERT(n + 2 < argc);
      resolution = int2(fwk::fromString<int>(argv[n + 1]), fwk::fromString<int>(argv[n + 2]));
      ASSERT(resolution.x >= 320 && resolution.y >= 200);
      n += 2;
    } else if(argument == "-full-screen") {
      gfx_flags |= GfxDeviceOpt::fullscreen;
    } else if(argument == "-no-vsync") {
      gfx_flags &= ~GfxDeviceOpt::vsync;
    } else if(argument == "-maximized") {
      gfx_flags |= GfxDeviceOpt::maximized;
    } else if(argument == "-spawn") {
      ASSERT(n + 3 < argc);
      int2 pos(fwk::fromString<int>(argv[n + 2]), fwk::fromString<int>(argv[n + 3]));
      spawns.emplace_back(SpawnCommand{argv[n + 1], pos, {}});
      n += 3;
    } else if(argument == "-spawn-to") {
      ASSERT(n + 5 < argc);
      int2 pos1(fwk::fromString<int>(argv[n + 2]), fwk::fromString<int>(argv[n + 3]));
      int2 pos2(fwk::fromString<int>(argv[n + 4]), fwk::fromString<int>(argv[n + 5]));
      spawns.emplace_back(SpawnCommand{argv[n + 1], pos1, pos2 - pos1});
      n += 5;
    } else if(argument == "-background") {
      ASSERT(n + 1 < argc);
      background = argv[++n];
    } else if(argument == "-zoom") {
      ASSERT(n + 1 < argc);
      zoom = fwk::fromString<float>(argv[++n]);
    } else if(argument == "-fixed-fps") {
      ASSERT(n + 1 < argc);
      fixedFps = fwk::fromString<int>(argv[++n]);
      ASSERT(fixedFps >= 1 && fixedFps <= 60);
    } else {
      printf("Unsupported argument: %s\n", argument.c_str());
      exit(1);
    }
  }

  GfxDevice gfx_device;
  gfx_device.createWindow("FXTester - particle system tester", resolution, gfx_flags, OpenglProfile::compatibility,
                          2.1);

  FXTester tester(zoom, fixedFps);
  for (auto spawn : spawns) {
    if (!tester.spawnEffect(spawn.name, spawn.pos, spawn.off)) {
      printf("Unknown effect: %s\n", spawn.name.c_str());
      exit(1);
    }
    tester.focusOn(spawn.pos);
  }

  if(!background.empty())
    if(!tester.setBackground(background)) {
      printf("Unknown background: %s\n", background.c_str());
      exit(1);
    }

  ::installOpenglDebugHandler();
  gfx_device.runMainLoop(FXTester::mainLoop, &tester);

  return 0;
}
}
}
