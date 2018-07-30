#include "imgui_wrapper.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <fwk/gfx/gfx_device.h>
#include <fwk/gfx/opengl.h>
#include <fwk/sys/input.h>
#include <fwk/sys/xml.h>

namespace fx::tester {

ImGuiWrapper *ImGuiWrapper::s_instance = nullptr;

ImGuiWrapper::ImGuiWrapper(GfxDevice &device, ImGuiStyleMode style_mode) {
  ASSERT("You can only create a single instance of ImGuiWrapper" && !s_instance);
  s_instance = this;

  // Build texture atlas
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr;

  static const ImWchar glyph_ranges[] = {
      0x0020, 0x00FF, // Basic Latin + Latin Supplement
      0x0100, 0x017F, // Latin Extended-A
      0x0180, 0x024F, // Latin Extended-B
      0,
  };

  io.Fonts->AddFontFromFileTTF("data/LiberationSans-Regular.ttf", style_mode == ImGuiStyleMode::mini ? 12 : 14, 0,
                               glyph_ranges);
  io.FontDefault = io.Fonts->Fonts.back();

  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

  // Upload texture to graphics system
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGenTextures(1, &m_font_tex);
  glBindTexture(GL_TEXTURE_2D, m_font_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

  // Store our identifier
  io.Fonts->TexID = (void *)(intptr_t)m_font_tex;

  // Restore state
  glBindTexture(GL_TEXTURE_2D, last_texture);

  io.KeyMap[ImGuiKey_Tab] = InputKey::tab;
  io.KeyMap[ImGuiKey_LeftArrow] = InputKey::left;
  io.KeyMap[ImGuiKey_RightArrow] = InputKey::right;
  io.KeyMap[ImGuiKey_UpArrow] = InputKey::up;
  io.KeyMap[ImGuiKey_DownArrow] = InputKey::down;
  io.KeyMap[ImGuiKey_PageUp] = InputKey::pageup;
  io.KeyMap[ImGuiKey_PageDown] = InputKey::pagedown;
  io.KeyMap[ImGuiKey_Home] = InputKey::home;
  io.KeyMap[ImGuiKey_End] = InputKey::end;
  io.KeyMap[ImGuiKey_Delete] = InputKey::del;
  io.KeyMap[ImGuiKey_Backspace] = InputKey::backspace;
  io.KeyMap[ImGuiKey_Enter] = InputKey::enter;
  io.KeyMap[ImGuiKey_Escape] = InputKey::esc;
  io.KeyMap[ImGuiKey_A] = 'a';
  io.KeyMap[ImGuiKey_C] = 'c';
  io.KeyMap[ImGuiKey_V] = 'v';
  io.KeyMap[ImGuiKey_X] = 'x';
  io.KeyMap[ImGuiKey_Y] = 'y';

  io.RenderDrawListsFn = nullptr;
  io.SetClipboardTextFn = setClipboardText;
  io.GetClipboardTextFn = getClipboardText;
  io.ClipboardUserData = nullptr;

  /*
#ifdef _WIN32
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	io.ImeWindowHandle = wmInfo.info.win.window;
#endif*/

  if(style_mode == ImGuiStyleMode::mini) {
    auto &style = ImGui::GetStyle();
    style.FramePadding = {2, 1};
    style.ItemSpacing = {3, 3};
  }
}

ImGuiWrapper::ImGuiWrapper(ImGuiWrapper &&rhs)
    : m_last_time(rhs.m_last_time), m_procs(move(rhs.m_procs)), m_font_tex(rhs.m_font_tex) {
  rhs.m_font_tex = 0;
  s_instance = this;
}

ImGuiWrapper::~ImGuiWrapper() {
  if(s_instance != this)
    return;

  s_instance = nullptr;

  if(m_font_tex) {
    glDeleteTextures(1, &m_font_tex);
    ImGui::GetIO().Fonts->TexID = 0;
    m_font_tex = 0;
  }
  ImGui::Shutdown();
}

void ImGuiWrapper::saveSettings(XmlNode xnode) const {
  auto &settings = GImGui->Settings;
  for(auto &elem : settings) {
    auto enode = xnode.addChild("window");
    enode.addAttrib("name", enode.own(elem.Name));
    enode.addAttrib("pos", (int2)elem.Pos);
    enode.addAttrib("size", (int2)elem.Size);
    enode.addAttrib("collapsed", elem.Collapsed, false);
  }
  xnode.addAttrib("hide", o_hide_menu, false);
}

void ImGuiWrapper::loadSettings(CXmlNode xnode) {
  auto &settings = GImGui->Settings;
  settings.clear();
  auto enode = xnode.child("window");
  while(enode) {
    auto name = enode.attrib("name");
    if(!anyOf(settings, [&](const ImGuiIniData &elem) { return Str(elem.Name) == name; })) {
      ImGuiIniData elem;
      elem.Name = ImStrdup(enode.attrib("name"));
      elem.Id = ImHash(elem.Name, 0);
      elem.Pos = enode.attrib<int2>("pos");
      elem.Size = enode.attrib<int2>("size");
      elem.Collapsed = xnode.attrib("collapsed", false);
      settings.push_back(elem);
    }
    enode.next();
  }
  o_hide_menu = xnode.attrib("hide", false);
}

void ImGuiWrapper::addProcess(ProcessFunc func, void *arg) { m_procs.emplace_back(func, arg); }

void ImGuiWrapper::removeProcess(ProcessFunc func, void *arg) {
  m_procs = filter(m_procs, [=](const Process &proc) { return proc.func != func || proc.arg != arg; });
}

const char *ImGuiWrapper::getClipboardText(void *) {
  static string buffer;
  buffer = GfxDevice::instance().clipboardText();
  return buffer.c_str();
}

void ImGuiWrapper::setClipboardText(void *, const char *text) { GfxDevice::instance().setClipboardText(text); }

void ImGuiWrapper::drawFrame(GfxDevice &device) {
  if(o_hide_menu)
    return;

  ImGui::Render();
  auto *draw_data = ImGui::GetDrawData();

  // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
  ImGuiIO &io = ImGui::GetIO();
  int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
  if(fb_width == 0 || fb_height == 0)
    return;
  draw_data->ScaleClipRects(io.DisplayFramebufferScale);

  glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context

  int max_va = 0;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_va);
  for(int n = 0; n < max_va; n++)
    glDisableVertexAttribArray(n);

  // We are using the OpenGL fixed pipeline to make the example code simpler to read!
  // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  GLint last_viewport[4];
  glGetIntegerv(GL_VIEWPORT, last_viewport);
  GLint last_scissor_box[4];
  glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glEnable(GL_TEXTURE_2D);

  // Setup viewport, orthographic projection matrix
  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

// Render command lists
#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE *)0)->ELEMENT))
  for(int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList *cmd_list = draw_data->CmdLists[n];
    const ImDrawVert *vtx_buffer = cmd_list->VtxBuffer.Data;
    const ImDrawIdx *idx_buffer = cmd_list->IdxBuffer.Data;
    glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                    (const GLvoid *)((const char *)vtx_buffer + OFFSETOF(ImDrawVert, pos)));
    glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                      (const GLvoid *)((const char *)vtx_buffer + OFFSETOF(ImDrawVert, uv)));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert),
                   (const GLvoid *)((const char *)vtx_buffer + OFFSETOF(ImDrawVert, col)));

    for(int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if(pcmd->UserCallback) {
        pcmd->UserCallback(cmd_list, pcmd);
      } else {
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
        glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w),
                  (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
        glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                       sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
      }
      idx_buffer += pcmd->ElemCount;
    }
  }
#undef OFFSETOF

  // Restore modified state
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
  glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
  testGlError("imgui_stuff");
}

void ImGuiWrapper::beginFrame(GfxDevice &device) {
  ImGuiIO &io = ImGui::GetIO();
  memset(io.KeysDown, 0, sizeof(io.KeysDown));
  memset(io.MouseDown, 0, sizeof(io.MouseDown));

  if(!o_hide_menu) {
    for(auto &event : device.inputEvents()) {
      if(isOneOf(event.type(), InputEvent::key_down, InputEvent::key_pressed))
        io.KeysDown[event.key()] = true;
      if(event.isMouseEvent()) {
        for(auto button : all<InputButton>())
          if(event.mouseButtonPressed(button) || event.mouseButtonDown(button))
            io.MouseDown[(int)button] = true;
      }
      if(event.keyChar())
        io.AddInputCharacter(event.keyChar()); // TODO: this is UTF16...
    }

    const auto &state = device.inputState();

    io.KeyShift = state.isKeyPressed(InputKey::lshift);
    io.KeyCtrl = state.isKeyPressed(InputKey::lctrl);
    io.KeyAlt = state.isKeyPressed(InputKey::lalt);

    // TODO: proper handling of mouse outside of current window
    // TODO: detecting short mouse clicks (single frame)
    io.MousePos = float2(state.mousePos());
    io.MouseWheel = state.mouseWheelMove();
  }

  device.showCursor(io.MouseDrawCursor && !o_hide_menu ? false : true);

  // TODO: io.AddInputCharactersUTF8(event->text.text);

  io.DisplaySize = (ImVec2)float2(device.windowSize());
  // TODO:
  //io.DisplayFramebufferScale =
  //	ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

  double current_time = getTime();
  io.DeltaTime = m_last_time > 0.0 ? (float)(current_time - m_last_time) : (float)(1.0f / 60.0f);
  m_last_time = current_time;
  ImGui::NewFrame();

  for(auto &proc : m_procs)
    proc.func(proc.arg);
}

vector<InputEvent> ImGuiWrapper::finishFrame(GfxDevice &device) {
  if(o_hide_menu)
    return device.inputEvents();

  vector<InputEvent> out;

  ImGuiIO &io = ImGui::GetIO();
  for(auto event : device.inputEvents()) {
    if(event.isKeyEvent() && !io.WantCaptureKeyboard && !io.WantTextInput)
      out.emplace_back(event);
    if(event.isMouseEvent() && !io.WantCaptureMouse)
      out.emplace_back(event);
  }
  return out;
}
}
