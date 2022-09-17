#include "pch.h"
#include "InputManagerUI.h"
#include "Ouroboros/Input/InputManager.h"


#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <rttr/type.h>
#include <rttr/variant.h>
#include <rttr/property.h>
#include <rttr/enumeration.h>

#include <string>
InputManagerUI::InputManagerUI()
{
}

InputManagerUI::~InputManagerUI()
{
}

void InputManagerUI::Show()
{
	for (auto& input : oo::InputManager::axes)
	{
		ImGui::Separator();
		
		bool opened = ImGui::TreeNodeEx(input.first.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen| ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::Separator();
		if (opened == false)
			continue;
		ImGui::PushID(input.first.c_str());

		std::string name = input.second.GetName();
		if (ImGui::InputText("Name", &name, ImGuiInputTextFlags_EnterReturnsTrue))
			input.second.SetName(name);
		
		int inputType = static_cast<int>(input.second.GetType());
		if (DrawInputTypeUI(inputType))
			input.second.SetType(static_cast<oo::InputAxis::InputType>(inputType));
		//buttons
		int key = static_cast<int>(input.second.GetNegativeButton());
		if (DrawKeyInputUI("Negative Button", key))
			input.second.SetNegativeButton(static_cast<oo::InputAxis::InputCode>(key));

		key = static_cast<int>(input.second.GetPositiveButton());
		if (DrawKeyInputUI("Positive Button", key))
			input.second.SetPositiveButton(static_cast<oo::InputAxis::InputCode>(key));

		key = static_cast<int>(input.second.GetNegativeAltButton());
		if (DrawKeyInputUI("Negative Alt Button", key))
			input.second.SetNegativeAltButton(static_cast<oo::InputAxis::InputCode>(key));

		key = static_cast<int>(input.second.GetPositiveAltButton());
		if (DrawKeyInputUI("Positive Alt Button", key))
			input.second.SetPositiveAltButton(static_cast<oo::InputAxis::InputCode>(key));

		unsigned pressesRequired = input.second.GetPressesRequired();
		if (ImGui::DragScalarN("Presses Required", ImGuiDataType_::ImGuiDataType_U32, &pressesRequired, 1, 0.1f))
			input.second.SetPressesRequired(pressesRequired);

		float maxGapTime = input.second.GetMaxGapTime();
		if (ImGui::DragFloat("Max Gap Time", &maxGapTime, 0.1f))
			input.second.SetMaxGapTime(maxGapTime);

		float holdDurationRequired = input.second.GetHoldDurationRequired();
		if (ImGui::DragFloat("Hold Duration Required", &holdDurationRequired, 0.1f))
			input.second.SetHoldDurationRequired(holdDurationRequired);
		
		ImGui::PopID();
		ImGui::Separator();
	}
	
}

bool InputManagerUI::DrawInputTypeUI(int& curr)
{
	rttr::enumeration e = rttr::type::get<oo::InputAxis::InputType>().get_enumeration();
	auto values = e.get_values();

	std::string name = (curr < 0) ? "Invalid" : e.value_to_name(oo::InputAxis::InputType(curr)).data();
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::InputText("Input Type", &name);
	ImGui::PopItemFlag();
	ImGui::SameLine();

	bool changed = false;
	ImGui::PushID("InputTypeUI");
	if (ImGui::SmallButton("+"))
	{
		ImGui::OpenPopup("DrawInputButtonPopup");
	}
	if (ImGui::BeginPopup("DrawInputButtonPopup", ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration))
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

bool InputManagerUI::DrawKeyInputUI(const std::string& UIname,int& curr)
{
	rttr::enumeration e = rttr::type::get<oo::input::KeyCode>().get_enumeration();
	auto values = e.get_values();
	std::string name = (curr < 0) ? "Invalid" : e.value_to_name(oo::input::KeyCode(curr)).data();
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
	if (ImGui::BeginPopup("DrawInputKeyButtonPopup", ImGuiWindowFlags_NoTitleBar| ImGuiWindowFlags_NoResize| ImGuiWindowFlags_NoCollapse))
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
