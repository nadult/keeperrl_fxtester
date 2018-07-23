#include "imgui/imgui.h"
#include <fwk/format.h>

namespace fx_tester {

using namespace fwk;

template <class Index> bool selectIndex(ZStr title, Index &value, CSpan<const char *> strings) {
	DASSERT(!strings.empty() && strings.inRange((int)value));

	ImGui::Text("%s", title.c_str());
	int width = ImGui::GetItemRectSize().x;
	ImGui::SameLine();
	ImGui::PushItemWidth(220 - width);
	int item = (int)value;
	bool ret = ImGui::Combo(format("##%", title).c_str(), &item, strings.data(), strings.size());
	ImGui::PopItemWidth();

	value = (Index)item;
	return ret;
}

template <class Enum, EnableIfEnum<Enum>...> bool selectEnum(ZStr title, Enum &value) {
	array<const char *, count<Enum>()> strings;
	for(auto val : all<Enum>())
		strings[(int)val] = fwk::toString(val);
	return selectIndex(title, value, strings);
}
}
