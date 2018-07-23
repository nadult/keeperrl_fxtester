#include "fx_tester_base.h"
#include "imgui/imgui.h"
#include <fwk/format.h>

template <class Index>
bool selectIndex(fwk::ZStr title, Index &value, fwk::CSpan<const char *> strings) {
	DASSERT(!strings.empty() && strings.inRange((int)value));

	ImGui::Text("%s", title.c_str());
	int width = ImGui::GetItemRectSize().x;
	ImGui::SameLine();
	ImGui::PushItemWidth(220 - width);
	int item = (int)value;
	bool ret =
		ImGui::Combo(fwk::format("##%", title).c_str(), &item, strings.data(), strings.size());
	ImGui::PopItemWidth();

	value = (Index)item;
	return ret;
}

template <class Enum, fwk::EnableIfEnum<Enum>...> bool selectEnum(fwk::ZStr title, Enum &value) {
	fwk::array<const char *, fwk::count<Enum>()> strings;
	for(auto val : fwk::all<Enum>())
		strings[(int)val] = fwk::toString(val);
	return selectIndex(title, value, strings);
}
