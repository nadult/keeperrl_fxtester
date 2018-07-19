#include "imgui/imgui.h"
#include "imgui_wrapper.h"
#include <fwk/filesystem.h>
#include <fwk/gfx/font_factory.h>
#include <fwk/gfx/gfx_device.h>
#include <fwk/gfx/opengl.h>
#include <fwk/gfx/renderer2d.h>
#include <fwk/sys/backtrace.h>
#include <fwk/sys/input.h>

class App {
  public:
	App()
		: m_viewport(GfxDevice::instance().windowSize()),
		  m_font(FontFactory().makeFont("data/LiberationSans-Regular.ttf", 14)),
		  m_imgui(GfxDevice::instance(), ImGuiStyleMode::mini) {}

	void doMenu() {
		ImGui::Begin("PSystem2D menu", nullptr,
					 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::SetWindowSize(m_menu_size);

		if(ImGui::BeginMenu("Exit")) {
			if(ImGui::MenuItem("I AM SURE!"))
				m_exit_please = true;
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

		GfxDevice::clearColor(ColorId::green);
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

	mutable float2 m_max_extents;
	FilePath m_data_path;
	Font m_font;
	bool m_exit_please = false;
};

extern "C" {
int main(int argc, char **argv) {
	double time = getTime();
	int2 resolution(1300, 800);
	GfxDeviceFlags gfx_flags;
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
