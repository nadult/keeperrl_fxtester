#include "imgui/imgui.h"
#include "imgui_wrapper.h"
#include <fwk/filesystem.h>
#include <fwk/gfx/dtexture.h>
#include <fwk/gfx/font_factory.h>
#include <fwk/gfx/gfx_device.h>
#include <fwk/gfx/material.h>
#include <fwk/gfx/opengl.h>
#include <fwk/gfx/renderer2d.h>
#include <fwk/math/box.h>
#include <fwk/sys/backtrace.h>
#include <fwk/sys/input.h>
#include <fwk/sys/stream.h>

#include "particle_system.h"

class App {
  public:
	struct Background {
		PTexture texture;
		int tile_size;
		string name;
	};

	void loadBackgrounds() {
		auto bkg_dir = "data/backgrounds/", bkg_suffix = ".png";
		for(auto element : findFiles(bkg_dir, bkg_suffix)) {
			string file_name = bkg_dir + element + bkg_suffix;
			Loader loader(file_name);
			int tile_size = 48;
			if(element.find("_24") == element.size() - 3)
				tile_size = 24;
			m_backgrounds.emplace_back(make_immutable<DTexture>(file_name, loader), tile_size,
									   element);
		}
	}

	static float tileScale(int tile_size) { return float(tile_size) / 24.0f; }

	App()
		: m_viewport(GfxDevice::instance().windowSize()),
		  m_font(FontFactory().makeFont("data/LiberationSans-Regular.ttf", 14)),
		  m_imgui(GfxDevice::instance(), ImGuiStyleMode::mini) {
		m_ps.addDefaults();
		m_ps.addInstance(0, float2());

		auto &pdefs = m_ps.particleDefs();
		for(int n = 0; n < pdefs.size(); n++) {
			auto &pdef = pdefs[n];
			string file_name = "data/" + pdef.texture_name;
			Loader loader(file_name);
			m_textures.emplace_back(make_immutable<DTexture>(pdef.texture_name, loader));
			m_materials.emplace_back(m_textures.back());
		}

		loadBackgrounds();
	}

	void doMenu() {
		ImGui::Begin("PSystem2D menu", nullptr,
					 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::SetWindowSize(m_menu_size);

		if(ImGui::InputFloat("scale", &m_scale))
			m_scale = clamp(m_scale, 0.1f, 20.0f);

		if(!m_backgrounds.empty())
			if(ImGui::BeginMenu("Select background")) {
				for(int n = 0; n < m_backgrounds.size(); n++) {
					auto &back = m_backgrounds[n];
					if(ImGui::MenuItem(back.name.c_str()))
						m_background_id = n;
				}
				ImGui::EndMenu();
			}

		ImGui::Separator();

		static bool show_test_window = 0;
		if(show_test_window) {
			ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
			ImGui::ShowTestWindow(&show_test_window);
		}

		m_menu_width = 120;
		m_menu_size = vmax(m_menu_size, int2(m_menu_width + 20, ImGui::GetCursorPosY()));

		ImGui::End();
	}

	void tick(GfxDevice &device, double time_diff) {
		FWK_PROFILE("tick");

		m_ps.simulate(time_diff);

		auto events = device.inputEvents();
		m_imgui.beginFrame(device);
		doMenu();
		events = m_imgui.finishFrame(device);

		for(auto event : events) {
			if(event.keyDown(InputKey::f11)) {
				auto &gfx_device = GfxDevice::instance();
				auto flags = gfx_device.isWindowFullscreen() ? GfxDeviceFlags()
															 : GfxDeviceOpt::fullscreen_desktop;
				gfx_device.setWindowFullscreen(flags);
			}
		}
	}

	void render() const {
		FWK_PROFILE("render");
		Renderer2D rlist2d(m_viewport);

		float2 center(m_viewport.size() / 2);

		GfxDevice::clearColor(FColor(0.1, 0.1, 0.1));

		if(!m_backgrounds.empty()) {
			auto &back = m_backgrounds[m_background_id];
			SimpleMaterial mat(back.texture);
			float2 size = float2(back.texture->size()) * 0.5f * m_scale / tileScale(back.tile_size);
			FRect rect(center - size, center + size);
			rlist2d.addFilledRect(rect, mat);
		}

		for(auto &quad : m_ps.genQuads()) {
			float2 positions[4];
			for(int n = 0; n < 4; n++)
				positions[n] = quad.positions[n] * m_scale + center;
			rlist2d.addQuads(positions, quad.tex_coords, quad.colors,
							 m_materials[quad.particle_def_id]);
		}
		rlist2d.render();
	}
	const IRect &viewport() const { return m_viewport; }

	bool mainLoop(GfxDevice &device) {
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

	static bool mainLoop(GfxDevice &device, void *this_ptr) {
		return ((App *)this_ptr)->mainLoop(device);
	}

  private:
	ImGuiWrapper m_imgui;

	IRect m_viewport;
	int m_menu_width;
	int2 m_menu_size;

	float m_scale = 2.0f;

	ParticleManager m_ps;
	vector<PTexture> m_textures;
	vector<SimpleMaterial> m_materials;

	vector<Background> m_backgrounds;
	int m_background_id = 0;

	mutable float2 m_max_extents;
	FilePath m_data_path;
	Font m_font;
	bool m_exit_please = false;
};

extern "C" {
int main(int argc, char **argv) {
	double time = getTime();
	int2 resolution(1300, 800);
	GfxDeviceFlags gfx_flags = GfxDeviceOpt::resizable;
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
	gfx_device.createWindow("PSystems2D - particle system tester", resolution, gfx_flags);

	App game_app;
	gfx_device.runMainLoop(App::mainLoop, &game_app);

	return 0;
}
}
