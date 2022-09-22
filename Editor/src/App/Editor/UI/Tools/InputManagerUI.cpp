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
	for (auto& input : oo::InputManager::GetAxes())
	{
		ImGui::Separator();
		
		bool opened = ImGui::TreeNodeEx(input.GetName().c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen| ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::Separator();
		if (opened == false)
			continue;
		ImGui::PushID(input.GetName().c_str());

		std::string name = input.GetName();
		if (ImGui::InputText("Name", &name, ImGuiInputTextFlags_EnterReturnsTrue))
			input.SetName(name);
		auto actualInputType = input.GetType();
		int inputType = static_cast<int>(input.GetType());
		if (DrawInputTypeUI(inputType))
			input.SetType(static_cast<oo::InputAxis::InputType>(inputType));

		if (inputType == static_cast<int>(oo::InputAxis::InputType::MouseMovement))
		{
			ImGui::PopID();
			ImGui::Separator();
			continue;
		}
			

		//buttons
		int key = static_cast<int>(input.GetSettings().negativeButton);
        if (DrawKeyInputUI("Negative Button", key, oo::InputAxis::InputType::MouseButton == actualInputType))
            input.GetSettings().negativeButton = static_cast<oo::InputAxis::InputCode>(key);

		key = static_cast<int>(input.GetSettings().positiveButton);
		if (DrawKeyInputUI("Positive Button", key,oo::InputAxis::InputType::MouseButton == actualInputType))
			input.GetSettings().positiveButton = static_cast<oo::InputAxis::InputCode>(key);

		key = static_cast<int>(input.GetSettings().negativeAltButton);
		if (DrawKeyInputUI("Negative Alt Button", key,oo::InputAxis::InputType::MouseButton == actualInputType))
            input.GetSettings().negativeAltButton = static_cast<oo::InputAxis::InputCode>(key);

		key = static_cast<int>(input.GetSettings().positiveAltButton);
		if (DrawKeyInputUI("Positive Alt Button", key, oo::InputAxis::InputType::MouseButton == actualInputType))
            input.GetSettings().positiveAltButton = static_cast<oo::InputAxis::InputCode>(key);

        unsigned pressesRequired = input.GetSettings().pressesRequired;
        if (ImGui::DragScalarN("Presses Required", ImGuiDataType_::ImGuiDataType_U32, &pressesRequired, 1, 0.1f))
            input.GetSettings().pressesRequired = pressesRequired;

        float maxGapTime = input.GetSettings().maxGapTime;
        if (ImGui::DragFloat("Max Gap Time", &maxGapTime, 0.1f))
            input.GetSettings().maxGapTime = maxGapTime;

        float holdDurationRequired = input.GetSettings().holdDurationRequired;
        if (ImGui::DragFloat("Hold Duration Required", &holdDurationRequired, 0.1f))
            input.GetSettings().holdDurationRequired = holdDurationRequired;
		//__________________________CONTROLLER______________________//
		
		key = static_cast<int>(input.GetControllerSettings().negativeButton);
		if (DrawKeyInputUI("Negative Controller", key, oo::InputAxis::InputType::MouseButton == actualInputType))
			input.GetControllerSettings().negativeButton = static_cast<oo::InputAxis::InputCode>(key);

		key = static_cast<int>(input.GetControllerSettings().positiveButton);
		if (DrawKeyInputUI("Positive Controller", key, oo::InputAxis::InputType::MouseButton == actualInputType))
			input.GetControllerSettings().positiveButton = static_cast<oo::InputAxis::InputCode>(key);

		key = static_cast<int>(input.GetControllerSettings().negativeAltButton);
		if (DrawKeyInputUI("Negative Alt Controller", key, oo::InputAxis::InputType::MouseButton == actualInputType))
			input.GetControllerSettings().negativeAltButton = static_cast<oo::InputAxis::InputCode>(key);

		key = static_cast<int>(input.GetControllerSettings().positiveAltButton);
		if (DrawKeyInputUI("Positive Alt Controller", key, oo::InputAxis::InputType::MouseButton == actualInputType))
			input.GetControllerSettings().positiveAltButton = static_cast<oo::InputAxis::InputCode>(key);

		unsigned controllerpressesRequired = input.GetControllerSettings().pressesRequired;
		if (ImGui::DragScalarN("Presses Required Controller", ImGuiDataType_::ImGuiDataType_U32, &controllerpressesRequired, 1, 0.1f))
			input.GetControllerSettings().pressesRequired = controllerpressesRequired;

		float controllermaxGapTime = input.GetControllerSettings().maxGapTime;
		if (ImGui::DragFloat("Max Gap Time Controller", &controllermaxGapTime, 0.1f))
			input.GetControllerSettings().maxGapTime = controllermaxGapTime;

		float controllerholdDurationRequired = input.GetControllerSettings().holdDurationRequired;
		if (ImGui::DragFloat("Hold Duration Required Controller", &controllerholdDurationRequired, 0.1f))
			input.GetControllerSettings().holdDurationRequired = controllerholdDurationRequired;

		ImGui::PopID();
		ImGui::Separator();
	}
	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	ImGui::Dummy({contentRegion.x * 0.25f,0});
	ImGui::SameLine();
	if (ImGui::Button("Add Input",  {contentRegion.x * 0.5f,50.0f}))
	{
		oo::InputManager::GetAxes().push_back(oo::InputAxis());
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

bool InputManagerUI::DrawKeyInputUI(const std::string& UIname,int& curr, bool mouse)
{
	rttr::enumeration e = (mouse ? rttr::type::get<oo::input::MouseCode>() : rttr::type::get<oo::input::KeyCode>()).get_enumeration();
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
