#include "fx_tester.h"

#include "fx_emitter_def.h"
#include "fx_manager.h"
#include "fx_particle_def.h"
#include "fx_spawner.h"

#include "imgui/imgui.h"
#include "imgui_funcs.h"
#include "imgui_wrapper.h"

#include <fwk/filesystem.h>
#include <fwk/gfx/dtexture.h>
#include <fwk/gfx/gfx_device.h>
#include <fwk/gfx/material.h>
#include <fwk/gfx/opengl.h>
#include <fwk/gfx/renderer2d.h>
#include <fwk/math/box.h>
#include <fwk/parse.h>
#include <fwk/sys/backtrace.h>
#include <fwk/sys/input.h>
#include <fwk/sys/stream.h>

namespace fx::tester {

static constexpr int tile_size = default_tile_size;

// ------------------------------------------------------------------------------------------------
// ------------------------------------- SPAWN TOOL -----------------------------------------------

struct FXTester::SpawnTool {
	void update(FXManager &mgr) {
		for(auto &spawner : spawners)
			spawner.update(mgr);
		for(int n = 0; n < spawners.size(); n++)
			if(spawners[n].is_dead) {
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

	void add(int2 pos) {
		spawners.emplace_back(type, pos, system_id);
		selection_id = spawners.size() - 1;
	}

	void select(int2 pos) {
		selection_id = -1;
		for(int n = 0; n < spawners.size(); n++)
			if(spawners[n].tile_pos == pos) {
				selection_id = n;
				break;
			}
	}

	Spawner *selection() { return selection_id == -1 ? nullptr : &spawners[selection_id]; }

	void remove(FXManager &mgr, int2 pos) {
		for(auto &spawner : spawners)
			if(spawner.tile_pos == pos)
				spawner.kill(mgr);
	}

	vector<Spawner> spawners;
	int selection_id = -1;

	SpawnerType type = SpawnerType::single;
	ParticleSystemDefId system_id = ParticleSystemDefId(0);
};

void FXTester::spawnToolMenu() {
	auto &tool = *m_spawn_tool;

	auto names = transform(m_ps->systemDefs(), [](const auto &def) { return def.name.c_str(); });
	selectIndex("New system", tool.system_id, names);
	selectIndex("Spawner type", tool.type, {"single", "repeated"});

	ImGui::Text("LMB: add spawner\ndel: remove spawners under cursor");
	ImGui::Text("LMB + ctrl: select\n");
	ImGui::Separator();

	if(auto *sel = tool.selection()) {
		float anim_time = 0.0f;
		int num_active = 0, num_total = 0;

		if(m_ps->alive(sel->instance_id)) {
			auto &ps = m_ps->get(sel->instance_id);
			num_active = ps.numActiveParticles();
			num_total = ps.numTotalParticles();
			anim_time = ps.anim_time;
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
	}
}

void FXTester::spawnToolInput(CSpan<InputEvent> events) {
	auto &tool = *m_spawn_tool;
	for(auto event : events) {
		if(event.keyDown(InputKey::del))
			tool.remove(*m_ps, m_selected_tile);
		if(event.mouseButtonDown(InputButton::left)) {
			if(event.mods() & InputModifier::lctrl)
				tool.select(m_selected_tile);
			else
				tool.add(m_selected_tile);
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
	auto &tool = *m_occlusion_tool;
	auto occ_dir = "data/occluders/", occ_suffix = ".png";
	for(auto element : findFiles(occ_dir, occ_suffix)) {
		string file_name = occ_dir + element + occ_suffix;
		tool.textures.emplace_back(loadTexture(file_name), element);
	}
}

void FXTester::removeOccluder(int2 pos) {
	auto &occluders = m_occlusion_tool->occluders;
	for(int n = 0; n < occluders.size(); n++)
		if(occluders[n].second == pos) {
			occluders[n] = occluders.back();
			occluders.pop_back();
			n--;
		}
}

void FXTester::occlusionToolMenu() {
	auto &tool = *m_occlusion_tool;
	auto names = transform(tool.textures, [](const auto &pair) { return pair.second.c_str(); });
	selectIndex("New occluder", tool.new_occluder_id, names);

	if(ImGui::Button("Clear occluders"))
		tool.occluders.clear();

	ImGui::Text("LMB: add occluder\ndel: remove occluder");
}

void FXTester::occlusionToolInput(CSpan<InputEvent> events) {
	auto &tool = *m_occlusion_tool;

	for(auto &event : events) {
		if(event.mouseButtonDown(InputButton::left) && !tool.textures.empty())
			tool.occluders.emplace_back(tool.new_occluder_id, m_selected_tile);
		if(event.keyDown(InputKey::del))
			removeOccluder(m_selected_tile);
	}
}

void FXTester::drawOccluders(Renderer2D &out) const {
	auto &tool = *m_occlusion_tool;
	auto tile_rect = FRect(tileToScreen(int2(1)));

	for(auto &occluder : tool.occluders) {
		auto tex = tool.textures[occluder.first].first;
		out.addFilledRect(tile_rect + tileToScreen(occluder.second), tex);
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

FXTester::FXTester(float zoom, Maybe<int> fixed_fps)
	: m_viewport(GfxDevice::instance().windowSize()), m_fixed_fps(fixed_fps) {
	m_imgui.emplace(GfxDevice::instance(), ImGuiStyleMode::mini);
	m_ps.emplace();

	for(auto &pdef : m_ps->particleDefs()) {
		string file_name = "data/particles/" + pdef.texture_name;
		Loader loader(file_name);
		m_particle_textures.emplace_back(make_immutable<DTexture>(pdef.texture_name, loader));
		m_particle_materials.emplace_back(m_particle_textures.back());
	}

	m_spawn_tool.emplace();
	m_occlusion_tool.emplace();

	m_cursor_tex = loadTexture("data/cursor.png");
	loadBackgrounds();
	loadOccluders();

	setZoom(float2(m_viewport.size()) * 0.25f, zoom);
}

bool FXTester::spawnEffect(string name, int2 pos) {
	int id = 0;
	for(auto &def : m_ps->systemDefs()) {
		if(def.name == name) {
			auto old_type = m_spawn_tool->type;
			m_spawn_tool->type = SpawnerType::repeated;
			m_spawn_tool->system_id = ParticleSystemDefId(id);
			m_spawn_tool->add(pos);
			m_spawn_tool->type = old_type;
			return true;
		}
		id++;
	}

	return false;
}

bool FXTester::setBackground(string name) {
	if(name == "disabled") {
		m_background_id = fwk::none;
		return true;
	}
	for(int n = 0; n < m_backgrounds.size(); n++)
		if(m_backgrounds[n].name == name) {
			m_background_id = n;
			return true;
		}
	return false;
}

float2 FXTester::screenToTile(float2 spos) const { return spos / float(tile_size * m_zoom); }
float2 FXTester::tileToScreen(int2 tpos) const { return float2(tpos) * float(tile_size) * m_zoom; }
float2 FXTester::tileToScreen(float2 tpos) const { return tpos * float(tile_size) * m_zoom; }

void FXTester::setZoom(float2 screen_pos, float zoom) {
	float2 old_pos = screenToTile(screen_pos);
	m_zoom = clamp(zoom, 0.25f, 10.0f);
	float2 new_pos = screenToTile(screen_pos);
	m_top_left_tile += old_pos - new_pos;
}

void FXTester::doMenu() {
	ImGui::Begin("FXTester", nullptr,
				 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::SetWindowSize(m_menu_size);
	static bool initialized = false;
	if(!initialized) {
		ImGui::SetWindowPos(ImVec2(5, 5));
		initialized = true;
	}

	selectEnum("Mode", m_mode);
	{
		float zoom = m_zoom;
		if(ImGui::InputFloat("Zoom", &zoom))
			setZoom(float2(m_viewport.size()) * 0.5f, clamp(zoom, 0.25f, 10.0f));
	}

	if(ImGui::InputFloat("Anim speed", &m_animation_speed))
		m_animation_speed = clamp(m_animation_speed, 0.0f, 100.0f);
	ImGui::Checkbox("Show cursor", &m_show_cursor);
	ImGui::Text("%s", format("Cursor: %", m_selected_tile).c_str());

	if(ImGui::Button("Select background"))
		ImGui::OpenPopup("select_back");
	if(ImGui::BeginPopup("select_back")) {
		if(ImGui::MenuItem("disabled", nullptr, !m_background_id))
			m_background_id = fwk::none;
		for(int n = 0; n < m_backgrounds.size(); n++) {
			auto &back = m_backgrounds[n];
			if(ImGui::MenuItem(back.name.c_str(), nullptr, m_background_id == n)) {
				m_background_id = n;
			}
		}
		ImGui::EndPopup();
	}

	ImGui::Separator();

	if(m_mode == Mode::spawn)
		spawnToolMenu();
	else
		occlusionToolMenu();

	static bool show_test_window = 0;
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}

	m_menu_width = 220;
	m_menu_size = vmax(m_menu_size, int2(m_menu_width + 20, ImGui::GetCursorPosY()));

	ImGui::End();
}

void FXTester::tick(GfxDevice &device, double time_diff) {
	m_ps->simulate(time_diff * m_animation_speed);

	auto events = device.inputEvents();
	m_imgui->beginFrame(device);
	doMenu();
	events = m_imgui->finishFrame(device);

	for(auto event : events) {
		if(event.keyDown(InputKey::f11)) {
			auto &gfx_device = GfxDevice::instance();
			auto flags = gfx_device.isWindowFullscreen() ? GfxDeviceFlags()
														 : GfxDeviceOpt::fullscreen_desktop;
			gfx_device.setWindowFullscreen(flags);
		}
		if(event.keyDown(InputKey::f1))
			m_mode = Mode::spawn;
		else if(event.keyDown(InputKey::f2))
			m_mode = Mode::occlusion;

		if(event.mouseButtonPressed(InputButton::right))
			m_top_left_tile -= screenToTile(float2(event.mouseMove()));
		if(event.isMouseOverEvent()) {
			float2 sel_tile = screenToTile((float2)event.mousePos()) + m_top_left_tile;
			m_selected_tile = (int2)vfloor(sel_tile);
			if(int zoom = event.mouseWheel())
				setZoom((float2)event.mousePos(), m_zoom * (zoom > 0 ? 1.25f : 0.8f));
		}
	}

	if(m_mode == Mode::spawn)
		spawnToolInput(events);
	else if(m_mode == Mode::occlusion)
		occlusionToolInput(events);

	m_spawn_tool->update(*m_ps);
}

void FXTester::drawCursor(Renderer2D &out, int2 tile_pos, FColor color) const {
	float2 sel_pos = float2(tile_pos * tile_size) - float2(2);
	auto sel_rect = FRect(sel_pos, sel_pos + float2(tile_size + 4)) * m_zoom;
	out.addFilledRect(sel_rect, SimpleMaterial(m_cursor_tex, color));
}

void FXTester::render() const {
	Renderer2D out(m_viewport);
	out.setViewPos(tileToScreen(m_top_left_tile));

	GfxDevice::clearColor(FColor(0.1, 0.1, 0.1));
	float tile_to_screen = m_zoom * float(tile_size);

	if(m_background_id) {
		auto &back = m_backgrounds[*m_background_id];
		auto size = float2(back.texture->size()) * tile_to_screen / float(back.tile_size);
		out.addFilledRect(FRect(size), back.texture);
	}

	for(auto &quad : m_ps->genQuads()) {
		float2 positions[4], tex_coords[4];
		for(int n = 0; n < 4; n++) {
			positions[n] = {quad.positions[n].x * m_zoom, quad.positions[n].y * m_zoom};
			tex_coords[n] = {quad.tex_coords[n].x, quad.tex_coords[n].y};
		}
		FColor color(fwk::IColor(quad.color.r, quad.color.g, quad.color.b, quad.color.a));

		array<FColor, 4> colors{{color, color, color, color}};
		out.addQuads(positions, tex_coords, colors, m_particle_materials[quad.particle_def_id]);
	}

	drawOccluders(out);

	if(m_show_cursor)
		drawCursor(out, m_selected_tile, FColor(ColorId::white, 0.2f));
	if(m_spawn_tool->selection_id != -1)
		drawCursor(out, m_spawn_tool->spawners[m_spawn_tool->selection_id].tile_pos,
				   FColor(ColorId::blue, 0.2f));

	out.render();
}

bool FXTester::mainLoop(GfxDevice &device) {
	double time = getTime();
	static double last_time = time - 1.0 / 60.0;
	double time_diff = time - last_time;
	last_time = time;

	if(m_fixed_fps) {
		double desired_frame_time = 1.0 / double(*m_fixed_fps);
		if(time_diff < desired_frame_time) {
			fwk::sleep(desired_frame_time - time_diff);
			last_time = getTime();
		}
		time_diff = desired_frame_time;
	} else {
		time_diff = clamp(time_diff, 1 / 240.0, 1 / 5.0);
	}

	m_viewport = IRect(device.windowSize());

	tick(device, time_diff);
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
		m_background_id = 0;
}

bool FXTester::mainLoop(GfxDevice &device, void *this_ptr) {
	return ((FXTester *)this_ptr)->mainLoop(device);
}

PTexture FXTester::loadTexture(string file_name) {
	Loader loader(file_name);
	return make_immutable<DTexture>(file_name, loader);
}

extern "C" {
int main(int argc, char **argv) {
	double time = getTime();
	int2 resolution(1300, 800);
	GfxDeviceFlags gfx_flags = GfxDeviceOpt::resizable | GfxDeviceOpt::vsync;
	Backtrace::t_default_mode = BacktraceMode::full;

	string spawn_effect, background;
	int2 spawn_pos;
	float zoom = 2.0f;
	Maybe<int> fixed_fps;

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
			spawn_effect = argv[++n];
			spawn_pos.x = fwk::fromString<int>(argv[++n]);
			spawn_pos.y = fwk::fromString<int>(argv[++n]);
		} else if(argument == "-background") {
			ASSERT(n + 1 < argc);
			background = argv[++n];
		} else if(argument == "-zoom") {
			ASSERT(n + 1 < argc);
			zoom = fwk::fromString<float>(argv[++n]);
		} else if(argument == "-fixed-fps") {
			ASSERT(n + 1 < argc);
			fixed_fps = fwk::fromString<int>(argv[++n]);
			ASSERT(fixed_fps >= 1 && fixed_fps <= 60);
		} else {
			printf("Unsupported argument: %s\n", argument.c_str());
			exit(1);
		}
	}

	GfxDevice gfx_device;
	gfx_device.createWindow("FXTester - particle system tester", resolution, gfx_flags);

	FXTester tester(zoom, fixed_fps);
	if(!spawn_effect.empty())
		if(!tester.spawnEffect(spawn_effect, spawn_pos)) {
			printf("Unknown effect: %s\n", spawn_effect.c_str());
			exit(1);
		}
	if(!background.empty())
		if(!tester.setBackground(background)) {
			printf("Unknown background: %s\n", background.c_str());
			exit(1);
		}

	gfx_device.runMainLoop(FXTester::mainLoop, &tester);

	return 0;
}
}
}
