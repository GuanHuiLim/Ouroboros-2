#pragma once
#include <string>
#include <rttr/enumeration.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
class InputManagerUI
{
public:
	InputManagerUI();
	~InputManagerUI();
	void Show();
private:
	bool DrawInputTypeUI(int& curr);
	bool DrawKeyInputUI(const std::string& name,int& curr,bool mouse);
	template<typename EnumType>
	bool DrawControllerInputUI(const std::string& name, int& curr);
};


template<typename EnumType>
inline bool InputManagerUI::DrawControllerInputUI(const std::string& UIname, int& curr)
{
	rttr::enumeration e = rttr::type::get<EnumType>().get_enumeration();
	auto values = e.get_values();
	auto names = e.get_names();
	std::string name = (curr < 0) ? "Invalid" : e.value_to_name(static_cast<EnumType>(curr)).data();
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::InputText(UIname.c_str(), &name);
	ImGui::PopItemFlag();
	ImGui::SameLine();

	bool changed = false;
	ImGui::PushID(UIname.c_str());
	if (ImGui::SmallButton("+"))
	{
		ImGui::OpenPopup("DrawInputKeyButtonPopup");
	}
	if (ImGui::BeginPopup("DrawInputKeyButtonPopup", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
	{
		for (auto& v : values)
		{
			if (ImGui::MenuItem(e.value_to_name(v).data()))
			{
				curr = v.to_int();
				ImGui::CloseCurrentPopup();
				changed = true;
				break;
			}
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();
	return changed;
}
