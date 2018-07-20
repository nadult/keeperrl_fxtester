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

FXTester::FXTester()
	: m_viewport(GfxDevice::instance().windowSize()),
	  m_font(FontFactory().makeFont("data/LiberationSans-Regular.ttf", 14)),
	  m_imgui(GfxDevice::instance(), ImGuiStyleMode::mini) {

	auto &pdefs = m_ps.particleDefs();
	for(int n = 0; n < pdefs.size(); n++) {
		auto &pdef = pdefs[n];
		string file_name = "data/" + pdef.texture_name;
		Loader loader(file_name);
		m_particle_textures.emplace_back(make_immutable<DTexture>(pdef.texture_name, loader));
		m_particle_materials.emplace_back(m_particle_textures.back());
	}

	m_marker_tex = loadTexture("data/marker.png");
	loadBackgrounds();
}

void FXTester::doSpawnToolMenu() {
	vector<const char *> names =
		transform(m_ps.systemDefs(), [](const ParticleSystemDef &def) { return def.name.c_str(); });
	selectIndex("New system", m_spawn_tool.system_id, names);
	selectEnum("Spawner type", m_spawn_tool.type);

	ImGui::Text("LMB: add spawner\ndel: remove spawners under cursor");
	ImGui::Text("Spawners: %d", m_spawners.size());
}

void FXTester::doMenu() {
	ImGui::Begin("FXTester", nullptr,
				 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::SetWindowSize(m_menu_size);

	if(ImGui::InputFloat("Zoom", &m_zoom))
		m_zoom = clamp(m_zoom, 0.25f, 10.0f);
	if(ImGui::InputFloat("Anim speed", &m_animation_speed))
		m_animation_speed = clamp(m_animation_speed, 0.0f, 100.0f);

	if(ImGui::Button("Select background"))
		ImGui::OpenPopup("select_back");
	if(ImGui::BeginPopup("select_back")) {
		if(ImGui::MenuItem("disabled", nullptr, !m_background_id))
			m_background_id = none;
		for(int n = 0; n < m_backgrounds.size(); n++) {
			auto &back = m_backgrounds[n];
			if(ImGui::MenuItem(back.name.c_str(), nullptr, m_background_id == n))
				m_background_id = n;
		}
		ImGui::EndPopup();
	}

	ImGui::Text("Alive systems: %d", m_ps.aliveSystems().size());
	ImGui::Separator();

	doSpawnToolMenu();

	static bool show_test_window = 0;
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}

	m_menu_width = 220;
	m_menu_size = vmax(m_menu_size, int2(m_menu_width + 20, ImGui::GetCursorPosY()));

	ImGui::End();
}

void FXTester::addSpawner(int2 pos) {
	auto &tool = m_spawn_tool;
	m_spawners.emplace_back(tool.type, m_selection, tool.system_id);
}

void FXTester::removeSpawner(int2 pos) {
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

void FXTester::tick(GfxDevice &device, double time_diff) {
	FWK_PROFILE("tick");

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
		if(event.keyDown(InputKey::del)) {
			removeSpawner(m_selection);
		}
		if(event.mouseButtonPressed(InputButton::right))
			m_view_pos -= float2(event.mouseMove()) * screen_to_tile;
		if(event.mouseButtonDown(InputButton::left))
			addSpawner(m_selection);
		if(event.isMouseOverEvent()) {
			m_selection = int2(m_view_pos + float2(event.mousePos()) * screen_to_tile);
			if(int zoom = event.mouseWheel())
				m_zoom = clamp(m_zoom * (zoom > 0 ? 1.25f : 0.8f), 0.25f, 10.0f);
		}
	}

	updateSpawners();
}

void FXTester::render() const {
	FWK_PROFILE("render");
	Renderer2D rlist2d(m_viewport);
	rlist2d.setViewPos(m_view_pos * float(tile_size) * m_zoom);

	GfxDevice::clearColor(FColor(0.1, 0.1, 0.1));
	float tile_to_screen = m_zoom * float(tile_size);

	if(m_background_id) {
		auto &back = m_backgrounds[*m_background_id];
		auto size = float2(back.texture->size()) * tile_to_screen / float(back.tile_size);
		rlist2d.addFilledRect(FRect(size), back.texture);
	}

	for(auto &quad : m_ps.genQuads()) {
		float2 positions[4];
		for(int n = 0; n < 4; n++)
			positions[n] = quad.positions[n] * m_zoom;
		array<FColor, 4> colors{{quad.color, quad.color, quad.color, quad.color}};
		rlist2d.addQuads(positions, quad.tex_coords, colors,
						 m_particle_materials[quad.particle_def_id]);
	}

	{
		float2 sel_pos(m_selection);
		FRect sel_rect = FRect(sel_pos, sel_pos + float2(1)) * tile_to_screen;
		rlist2d.addFilledRect(sel_rect, m_marker_tex);
	}

	rlist2d.render();
}

bool FXTester::mainLoop(GfxDevice &device) {
	double time = getTime();
	static double last_time = time - 1.0 / 60.0;
	double time_diff = time - last_time;
	last_time = time;
	time_diff = clamp(time_diff, 1 / 240.0, 1 / 5.0);

	if(auto *profiler = Profiler::instance())
		profiler->updateTimer("mainLoop", time - device.frameTime(), time, false);

	m_viewport = IRect(device.windowSize());

	tick(device, time_diff);
	render();
	m_imgui.drawFrame(GfxDevice::instance());

	return !m_exit_please;
}

bool FXTester::mainLoop(GfxDevice &device, void *this_ptr) {
	return ((FXTester *)this_ptr)->mainLoop(device);
}

PTexture FXTester::loadTexture(string file_name) {
	Loader loader(file_name);
	return make_immutable<DTexture>(file_name, loader);
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

extern "C" {
int main(int argc, char **argv) {
	double time = getTime();
	int2 resolution(1300, 800);
	GfxDeviceFlags gfx_flags = GfxDeviceOpt::resizable | GfxDeviceOpt::vsync;
	Backtrace::t_default_mode = BacktraceMode::full;

	for(int n = 1; n < argc; n++) {
		string argument = argv[n];
		if(argument == "--res") {
			ASSERT(n + 2 < argc);
			resolution = int2(fromString<int>(argv[n + 1]), fromString<int>(argv[n + 2]));
			ASSERT(resolution.x >= 320 && resolution.y >= 200);
			n += 2;
		} else if(argument == "--full-screen") {
			gfx_flags |= GfxDeviceOpt::fullscreen;
		} else if(argument == "--no-vsync") {
			gfx_flags &= ~GfxDeviceOpt::vsync;
		} else if(argument == "--maximized") {
			gfx_flags |= GfxDeviceOpt::maximized;
		} else {
			FATAL("Unsupported argument: %s", argument.c_str());
		}
	}

	GfxDevice gfx_device;
	gfx_device.createWindow("FXTester - particle system tester", resolution, gfx_flags);

	FXTester tester;
	gfx_device.runMainLoop(FXTester::mainLoop, &tester);

	return 0;
}
}
