#pragma once

#include "base.h"

DEFINE_ENUM(ImGuiStyleMode, normal, mini);

class ImGuiWrapper {
  public:
	ImGuiWrapper(GfxDevice &, ImGuiStyleMode);
	ImGuiWrapper(ImGuiWrapper &&);
	~ImGuiWrapper();

	void operator=(const ImGuiWrapper &) = delete;

	static ImGuiWrapper *instance() { return s_instance; }

	bool o_hide_menu = false;

	void beginFrame(GfxDevice &);
	vector<InputEvent> finishFrame(GfxDevice &);
	void drawFrame(GfxDevice &);

	void saveSettings(XmlNode) const;
	void loadSettings(CXmlNode);

	using ProcessFunc = void (*)(void *);
	void addProcess(ProcessFunc, void *arg);
	void removeProcess(ProcessFunc, void *arg);

  private:
	static const char *getClipboardText(void *);
	static void setClipboardText(void *, const char *);

	struct Process {
		ProcessFunc func;
		void *arg;
	};

	double m_last_time = -1.0;
	vector<Process> m_procs;
	uint m_font_tex = 0;
	static ImGuiWrapper *s_instance;
};
