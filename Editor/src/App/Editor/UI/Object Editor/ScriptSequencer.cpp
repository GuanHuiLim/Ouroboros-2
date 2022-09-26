#include "pch.h"
#include "ScriptSequencer.h"
#include <Ouroboros/Scripting/ScriptManager.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "App/Editor/UI/Tools/WarningMessage.h"
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
	static bool dragging = false;
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
				//auto& afterlist = oo::ScriptManager::GetAfterDefaultOrder();
				//auto iter = std::find(afterlist.begin(), afterlist.end(), sci);
				//afterlist.erase(iter);
                try
                {
                    oo::ScriptManager::RemoveBeforeDefaultOrder(sci);
                    oo::ScriptManager::InsertBeforeDefaultOrder(sci);
                }
                catch (std::exception const& e)
                {
                    WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, e.what());
                }
			}
			payload = ImGui::AcceptDragDropPayload("DEFAULT");
			if (payload)
			{
				oo::ScriptClassInfo sci = *static_cast<oo::ScriptClassInfo*>(payload->Data);
                try
                {
                    oo::ScriptManager::InsertBeforeDefaultOrder(sci);
                }
                catch (std::exception const& e)
                {
                    WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, e.what());
                }
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::SetCursorPos(pos);
		int counter = 0;
		for (auto& item : oo::ScriptManager::GetBeforeDefaultOrder())
		{
			ImGui::PushID(counter);
			if (dragging)
			{
				ImVec2 curr_cursor_pos = ImGui::GetCursorPos();
				ImGui::Separator();
				ImGui::SetCursorPos(curr_cursor_pos);
				ImGui::Selectable("##s_line", false, ImGuiSelectableFlags_Disabled, { 0,5.0f });
				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BEFORE");
					if (payload)
					{
						oo::ScriptClassInfo sci = *static_cast<oo::ScriptClassInfo*>(payload->Data);
						try
						{
							oo::ScriptManager::RemoveBeforeDefaultOrder(sci);
							oo::ScriptManager::InsertBeforeDefaultOrder(sci, counter);
						}
						catch (std::exception const& e)
						{
							WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, e.what());
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
			if (ImGui::SmallButton("X"))
			{
                try
                {
                    oo::ScriptManager::RemoveBeforeDefaultOrder(item);
                }
                catch (std::exception const& e)
                {
                    LOG_TRACE(e.what());
                }
				//auto& beforelist = oo::ScriptManager::GetBeforeDefaultOrder();
				//auto iter = std::find(beforelist.begin(), beforelist.end(), item);
				//if(iter != beforelist.end())
				//	beforelist.erase(iter);
			}
			ImGui::SameLine();
			ImGui::Selectable(item.ToString().c_str(), false);
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload))
			{
				ImGui::SetDragDropPayload("BEFORE", &item, sizeof(oo::ScriptClassInfo));
					ImGui::Text(item.ToString().c_str());
				ImGui::EndDragDropSource();
			}
			
			++counter;
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
				//auto& beforelist = oo::ScriptManager::GetBeforeDefaultOrder();
				//auto iter = std::find(beforelist.begin(), beforelist.end(), sci);
				//beforelist.erase(iter);
                try
                {
                    oo::ScriptManager::RemoveAfterDefaultOrder(sci);
                    oo::ScriptManager::InsertAfterDefaultOrder(sci);
                }
                catch (std::exception const& e)
                {
                    WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, e.what());
                }
			}
			payload = ImGui::AcceptDragDropPayload("DEFAULT");
			if (payload)
			{
				oo::ScriptClassInfo sci = *static_cast<oo::ScriptClassInfo*>(payload->Data);
                try
                {
                    oo::ScriptManager::InsertAfterDefaultOrder(sci);
                }
                catch (std::exception const& e)
                {
                    WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, e.what());
                }
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SetCursorPos(pos);
		int counter = 0;
		for (auto& item : oo::ScriptManager::GetAfterDefaultOrder())
		{
			ImGui::PushID(counter);
			if (dragging)
			{
				ImVec2 curr_cursor_pos = ImGui::GetCursorPos();
				ImGui::Separator();
				ImGui::SetCursorPos(curr_cursor_pos);
				ImGui::Selectable("##s_line", false, ImGuiSelectableFlags_Disabled, { 0,5.0f });
				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AFTER");
					if (payload)
					{
						oo::ScriptClassInfo sci = *static_cast<oo::ScriptClassInfo*>(payload->Data);
						try
						{
							oo::ScriptManager::RemoveAfterDefaultOrder(sci);
							oo::ScriptManager::InsertAfterDefaultOrder(sci, counter);
						}
						catch (std::exception const& e)
						{
							WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, e.what());
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
			if (ImGui::SmallButton("X"))
			{
                try
                {
                    oo::ScriptManager::RemoveAfterDefaultOrder(item);
                }
                catch (std::exception const& e)
                {
                    WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, e.what());
                }
				//auto& afterlist = oo::ScriptManager::GetAfterDefaultOrder();
				//auto iter = std::find(afterlist.begin(), afterlist.end(), item);
				//if (iter != afterlist.end())
				//	afterlist.erase(iter);
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
			++counter;
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

	{
		ImRect r = ImGui::GetCurrentWindowRead()->Rect();
		dragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::IsMouseHoveringRect(r.Min, r.Max);
	}
}
