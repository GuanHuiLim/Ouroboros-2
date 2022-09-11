#include "pch.h"
#include "ScriptSequencer.h"
#include <Ouroboros/Scripting/ScriptManager.h>
#include <imgui/imgui.h>
ScriptSequencer::ScriptSequencer()
{
}

ScriptSequencer::~ScriptSequencer()
{
}

void ScriptSequencer::Show()
{
	ImVec2 region = ImGui::GetContentRegionAvail();
	ImVec2 childWindowSize = { region.x * 0.9f, region.y * 0.45f };

	{
		ImGui::Text("Before Internal Update :");
		ImGui::BeginChild("before", childWindowSize, true);
		ImGui::Dummy({ 5,0 }); ImGui::SameLine();
		ImGui::BeginGroup();
		ImVec2 pos = ImGui::GetCursorPos();
		ImGui::Selectable("##beforeList", false, ImGuiSelectableFlags_Disabled, { childWindowSize.x,childWindowSize.y * 0.90f });
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AFTER");
			if (payload)
			{
				oo::ScriptClassInfo sci = **static_cast<oo::ScriptClassInfo**>(payload->Data);
				auto& afterlist = oo::ScriptManager::GetAfterDefaultOrder();
				auto iter = std::find(afterlist.begin(), afterlist.end(), sci);
				afterlist.erase(iter);
				oo::ScriptManager::GetBeforeDefaultOrder().emplace_back(sci);
			}
		}
		ImGui::SetCursorPos(pos);
		for (auto& item : oo::ScriptManager::GetBeforeDefaultOrder())
		{
			ImGui::Selectable(item.ToString().c_str(), false);
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload))
			{
				ImGui::SetDragDropPayload("BEFORE", &item, sizeof(oo::ScriptClassInfo));
				ImGui::Text(item.ToString().c_str());
				ImGui::EndDragDropSource();
			}
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
		ImGui::Selectable("##afterList", false, ImGuiSelectableFlags_Disabled, { childWindowSize.x ,childWindowSize.y* 0.90f });
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BEFORE");
			if (payload)
			{
				oo::ScriptClassInfo sci = **static_cast<oo::ScriptClassInfo**>(payload->Data);
				auto& beforelist = oo::ScriptManager::GetBeforeDefaultOrder();
				auto iter = std::find(beforelist.begin(), beforelist.end(), sci);
				beforelist.erase(iter);
				oo::ScriptManager::GetAfterDefaultOrder().emplace_back(sci);
			}
		}
		ImGui::SetCursorPos(pos);
		for (auto& item : oo::ScriptManager::GetAfterDefaultOrder())
		{
			ImGui::Selectable(item.ToString().c_str(), false);
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload))
			{
				ImGui::SetDragDropPayload("AFTER", &item, sizeof(oo::ScriptClassInfo));
				ImGui::Text(item.ToString().c_str());
				ImGui::EndDragDropSource();
			}
		}
		ImGui::EndGroup();
		ImGui::EndChild();
	}
}
