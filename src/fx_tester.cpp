#include "fx_tester.h"

#include "imgui/imgui.h"
#include "imgui_funcs.h"
#include "imgui_wrapper.h"
#include <fwk/gfx/dtexture.h>
#include <fwk/gfx/gfx_device.h>
#include <fwk/gfx/material.h>
#include <fwk/gfx/opengl.h>
#include <fwk/gfx/renderer2d.h>
#include <fwk/math/box.h>
#include <fwk/sys/backtrace.h>
#include <fwk/sys/input.h>
#include <fwk/sys/stream.h>

#include "fx_manager.h"
#include "spawner.h"

using namespace fwk;

// TODO: move it to separate file
Color::Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a) : SDL_Color{r, g, b, a} {}

// ------------------------------------------------------------------------------------------------
// ------------------------------------- SPAWN TOOL -----------------------------------------------

struct FXTester::SpawnTool {
	SpawnerType type = SpawnerType::single;
	ParticleSystemDefId system_id = ParticleSystemDefId(0);
};

void FXTester::spawnToolMenu() {
	auto &tool = *m_spawn_tool;

	auto names =
		transform(m_ps.systemDefs(), [](const ParticleSystemDef &def) { return def.name.c_str(); });
	selectIndex("New system", tool.system_id, names);
	selectEnum("Spawner type", tool.type);

	ImGui::Text("LMB: add spawner\ndel: remove spawners under cursor");
	ImGui::Text("Spawners: %d", m_spawners.size());
}

void FXTester::addSpawner(fwk::int2 pos) {
	auto &tool = *m_spawn_tool;
	m_spawners.emplace_back(tool.type, m_selected_tile, tool.system_id);
}

void FXTester::removeSpawner(fwk::int2 pos) {
	for(auto &spawner : m_spawners)
		if(spawner.tile_pos == pos)
			spawner.kill(m_ps);
}

void FXTester::updateSpawners() {
	for(auto &spawner : m_spawners)
		spawner.update(m_ps);
	for(int n = 0; n < m_spawners.size(); n++)
		if(m_spawners[n].is_dead) {
			m_spawners[n] = m_spawners.back();
			m_spawners.pop_back();
			n--;
		}
}

void FXTester::spawnToolInput(CSpan<InputEvent> events) {
	for(auto event : events) {
		if(event.keyDown(InputKey::del))
			removeSpawner(m_selected_tile);
		if(event.mouseButtonDown(InputButton::left))
			addSpawner(m_selected_tile);
	}
}

// ------------------------------------------------------------------------------------------------
// ----------------------------------- OCCLUSION TOOL ---------------------------------------------

struct FXTester::OcclusionTool {
	fwk::vector<pair<PTexture, string>> textures;
	fwk::vector<pair<int, fwk::int2>> occluders;
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
	}
}

void FXTester::drawOccluders(Renderer2D &out) const {
	auto &tool = *m_occlusion_tool;

	float tile_to_screen = m_zoom * tile_size;
	FRect tile_rect = FRect(fwk::float2(tile_to_screen));

	for(auto &occluder : tool.occluders) {
		auto tex = tool.textures[occluder.first].first;
		out.addFilledRect(tile_rect + fwk::float2(occluder.second) * tile_to_screen, tex);
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

FXTester::FXTester()
	: m_viewport(GfxDevice::instance().windowSize()),
	  m_imgui(GfxDevice::instance(), ImGuiStyleMode::mini) {

	auto &pdefs = m_ps.particleDefs();
	for(int n = 0; n < pdefs.size(); n++) {
		auto &pdef = pdefs[n];
		string file_name = "data/particles/" + pdef.texture_name;
		Loader loader(file_name);
		m_particle_textures.emplace_back(make_immutable<DTexture>(pdef.texture_name, loader));
		m_particle_materials.emplace_back(m_particle_textures.back());
	}

	m_spawn_tool.emplace();
	m_occlusion_tool.emplace();

	m_marker_tex = loadTexture("data/marker.png");
	loadBackgrounds();
	loadOccluders();
}

void FXTester::doMenu() {
	ImGui::Begin("FXTester", nullptr,
				 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::SetWindowSize(m_menu_size);

	selectEnum("Mode", m_mode);
	if(ImGui::InputFloat("Zoom", &m_zoom))
		m_zoom = fwk::clamp(m_zoom, 0.25f, 10.0f);
	if(ImGui::InputFloat("Anim speed", &m_animation_speed))
		m_animation_speed = fwk::clamp(m_animation_speed, 0.0f, 100.0f);
	ImGui::Checkbox("Show cursor", &m_show_cursor);

	if(ImGui::Button("Select background"))
		ImGui::OpenPopup("select_back");
	if(ImGui::BeginPopup("select_back")) {
		if(ImGui::MenuItem("disabled", nullptr, !m_background_id))
			m_background_id = fwk::none;
		for(int n = 0; n < m_backgrounds.size(); n++) {
			auto &back = m_backgrounds[n];
			if(ImGui::MenuItem(back.name.c_str(), nullptr, m_background_id == n))
				m_background_id = n;
		}
		ImGui::EndPopup();
	}

	ImGui::Text("Alive systems: %d", m_ps.aliveSystems().size());
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
	m_menu_size = vmax(m_menu_size, fwk::int2(m_menu_width + 20, ImGui::GetCursorPosY()));

	ImGui::End();
}

void FXTester::tick(GfxDevice &device, double time_diff) {
	m_ps.simulate(time_diff * m_animation_speed);

	auto events = device.inputEvents();
	m_imgui.beginFrame(device);
	doMenu();
	events = m_imgui.finishFrame(device);

	float screen_to_tile = 1.0f / (m_zoom * tile_size);

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
			m_view_pos -= fwk::float2(event.mouseMove()) * screen_to_tile;
		if(event.isMouseOverEvent()) {
			m_selected_tile =
				fwk::int2(m_view_pos + fwk::float2(event.mousePos()) * screen_to_tile);
			if(int zoom = event.mouseWheel())
				m_zoom = fwk::clamp(m_zoom * (zoom > 0 ? 1.25f : 0.8f), 0.25f, 10.0f);
		}
	}

	if(m_mode == Mode::spawn)
		spawnToolInput(events);
	else if(m_mode == Mode::occlusion)
		occlusionToolInput(events);

	updateSpawners();
}

void FXTester::render() const {
	Renderer2D out(m_viewport);
	out.setViewPos(m_view_pos * float(tile_size) * m_zoom);

	GfxDevice::clearColor(FColor(0.1, 0.1, 0.1));
	float tile_to_screen = m_zoom * float(tile_size);

	if(m_background_id) {
		auto &back = m_backgrounds[*m_background_id];
		auto size = fwk::float2(back.texture->size()) * tile_to_screen / float(back.tile_size);
		out.addFilledRect(FRect(size), back.texture);
	}

	for(auto &quad : m_ps.genQuads()) {
		fwk::float2 positions[4], tex_coords[4];
		for(int n = 0; n < 4; n++) {
			positions[n] = {quad.positions[n].x * m_zoom, quad.positions[n].y * m_zoom};
			tex_coords[n] = {quad.tex_coords[n].x, quad.tex_coords[n].y};
		}
		FColor color(IColor(quad.color.r, quad.color.g, quad.color.b, quad.color.a));

		array<FColor, 4> colors{{color, color, color, color}};
		out.addQuads(positions, tex_coords, colors, m_particle_materials[quad.particle_def_id]);
	}

	drawOccluders(out);

	if(m_show_cursor) {
		fwk::float2 sel_pos(m_selected_tile);
		FRect sel_rect = FRect(sel_pos, sel_pos + fwk::float2(1)) * tile_to_screen;
		out.addFilledRect(sel_rect, m_marker_tex);
	}

	out.render();
}

bool FXTester::mainLoop(GfxDevice &device) {
	double time = getTime();
	static double last_time = time - 1.0 / 60.0;
	double time_diff = time - last_time;
	last_time = time;
	time_diff = fwk::clamp(time_diff, 1 / 240.0, 1 / 5.0);

	m_viewport = IRect(device.windowSize());

	tick(device, time_diff);
	render();
	m_imgui.drawFrame(GfxDevice::instance());

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
	fwk::int2 resolution(1300, 800);
	GfxDeviceFlags gfx_flags = GfxDeviceOpt::resizable | GfxDeviceOpt::vsync;
	Backtrace::t_default_mode = BacktraceMode::full;

	for(int n = 1; n < argc; n++) {
		string argument = argv[n];
		if(argument == "--res") {
			ASSERT(n + 2 < argc);
			resolution = fwk::int2(fromString<int>(argv[n + 1]), fromString<int>(argv[n + 2]));
			ASSERT(resolution.x >= 320 && resolution.y >= 200);
			n += 2;
		} else if(argument == "--full-screen") {
			gfx_flags |= GfxDeviceOpt::fullscreen;
		} else if(argument == "--no-vsync") {
			gfx_flags &= ~GfxDeviceOpt::vsync;
		} else if(argument == "--maximized") {
			gfx_flags |= GfxDeviceOpt::maximized;
		} else {
			printf("Unsupported argument: %s\n", argument.c_str());
			exit(1);
		}
	}

	GfxDevice gfx_device;
	gfx_device.createWindow("FXTester - particle system tester", resolution, gfx_flags);

	FXTester tester;
	gfx_device.runMainLoop(FXTester::mainLoop, &tester);

	return 0;
}
}
