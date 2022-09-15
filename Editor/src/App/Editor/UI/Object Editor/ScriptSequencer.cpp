#include "pch.h"
#include "ScriptSequencer.h"
#include <Ouroboros/Scripting/ScriptManager.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
ScriptSequencer::ScriptSequencer()
{
}

ScriptSequencer::~ScriptSequencer()
{
}

void ScriptSequencer::Show()
{
	ImVec2 region = ImGui::GetContentRegionAvail();
	ImVec2 childWindowSize = { region.x * 0.6f, region.y * 0.45f };
	ImGui::BeginGroup();
	{
		ImGui::Text("Before Internal Update :");
		ImGui::BeginChild("before", childWindowSize, true);
		ImGui::Dummy({ 5,0 }); ImGui::SameLine();
		ImGui::BeginGroup();
		ImVec2 pos = ImGui::GetCursorPos();
		ImGui::Selectable("##beforeList", false, ImGuiSelectableFlags_Disabled| ImGuiSelectableFlags_AllowItemOverlap, { childWindowSize.x,childWindowSize.y * 0.90f });
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AFTER");
			if (payload)
			{
				oo::ScriptClassInfo sci = *static_cast<oo::ScriptClassInfo*>(payload->Data);
				auto& afterlist = oo::ScriptManager::GetAfterDefaultOrder();
				auto iter = std::find(afterlist.begin(), afterlist.end(), sci);
				afterlist.erase(iter);
				oo::ScriptManager::GetBeforeDefaultOrder().emplace_back(sci);
			}
			payload = ImGui::AcceptDragDropPayload("DEFAULT");
			if (payload)
			{
				oo::ScriptClassInfo sci = *static_cast<oo::ScriptClassInfo*>(payload->Data);
				oo::ScriptManager::GetBeforeDefaultOrder().emplace_back(sci);
			}
		}
		ImGui::SetCursorPos(pos);
		int counter = 0;
		for (auto& item : oo::ScriptManager::GetBeforeDefaultOrder())
		{
			ImGui::PushID(++counter);
			if (ImGui::SmallButton("X"))
			{
				auto& beforelist = oo::ScriptManager::GetBeforeDefaultOrder();
				auto iter = std::find(beforelist.begin(), beforelist.end(), item);
				if(iter != beforelist.end())
					beforelist.erase(iter);
			}
			ImGui::SameLine();
			ImGui::Selectable(item.ToString().c_str(), false);
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload))
			{
				ImGui::SetDragDropPayload("BEFORE", &item, sizeof(oo::ScriptClassInfo));
					ImGui::Text(item.ToString().c_str());
				ImGui::EndDragDropSource();
			}
			ImGui::PopID();
		}
		ImGui::EndGroup();
		ImGui::EndChild();
	}

	{
		ImGui::Text("After Internal Update :");
		ImGui::BeginChild("after", childWindowSize, true);
		ImGui::Dummy({ 5,0 }); ImGui::SameLine();
		ImGui::BeginGroup();
		ImVec2 pos = ImGui::GetCursorPos();
		ImGui::Selectable("##afterList", false, ImGuiSelectableFlags_Disabled| ImGuiSelectableFlags_AllowItemOverlap, { childWindowSize.x ,childWindowSize.y* 0.90f });
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BEFORE");
			if (payload)
			{
				oo::ScriptClassInfo sci = *static_cast<oo::ScriptClassInfo*>(payload->Data);
				auto& beforelist = oo::ScriptManager::GetBeforeDefaultOrder();
				auto iter = std::find(beforelist.begin(), beforelist.end(), sci);
				beforelist.erase(iter);
				oo::ScriptManager::GetAfterDefaultOrder().emplace_back(sci);
			}
			payload = ImGui::AcceptDragDropPayload("DEFAULT");
			if (payload)
			{
				oo::ScriptClassInfo sci = *static_cast<oo::ScriptClassInfo*>(payload->Data);
				oo::ScriptManager::GetAfterDefaultOrder().emplace_back(sci);
			}
		}
		ImGui::SetCursorPos(pos);
		int counter = 0;
		for (auto& item : oo::ScriptManager::GetAfterDefaultOrder())
		{
			ImGui::PushID(++counter);
			if (ImGui::SmallButton("X"))
			{
				auto& afterlist = oo::ScriptManager::GetAfterDefaultOrder();
				auto iter = std::find(afterlist.begin(), afterlist.end(), item);
				if (iter != afterlist.end())
					afterlist.erase(iter);
			}
			ImGui::SameLine();
			ImGui::Selectable(item.ToString().c_str(), false);
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload))
			{
				ImGui::SetDragDropPayload("AFTER", &item, sizeof(oo::ScriptClassInfo));
				ImGui::Text(item.ToString().c_str());
				ImGui::EndDragDropSource();
			}
			ImGui::PopID();
		}
		ImGui::EndGroup();
		ImGui::EndChild();
	}
	ImGui::EndGroup();
	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Script Lists");
	ImGui::BeginChild("##scipt lists", { 0,0 }, true);
	for (auto& scripts : oo::ScriptManager::GetScriptList())
	{
		ImGui::Selectable(scripts.ToString().c_str(), false);
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload))
		{
			ImGui::SetDragDropPayload("DEFAULT", &scripts, sizeof(oo::ScriptClassInfo));
			ImGui::Text(scripts.ToString().c_str());
			ImGui::EndDragDropSource();
		}
	}
	ImGui::EndChild();
	ImGui::EndGroup();
}
