#include <pch.h>
#include "Hierarchy.h"
#include "App/Editor/Utility/ImGuiManager.h"

#include <stack>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <Ouroboros/Core/KeyCode.h>
#include <Ouroboros/ECS/GameObject.h>
#include <Scenegraph/include/Scenegraph.h>
#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/Scene/Scene.h>

Hierarchy::Hierarchy()
{
}

void Hierarchy::Show()
{
	if (m_isDragging && !ImGui::IsMouseDragging(ImGuiMouseButton_::ImGuiMouseButton_Left))
		m_isDragging = false;//false if not dragging
	bool found_dragging = false;

	scenegraph instance = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->GetGraph();//the scene graph should be obtained instead.
	ImGui::BeginChild("search bar", { 0,40 }, false);
	SearchFilter();
	ImGui::EndChild();

	scenenode::shared_pointer root_node = instance.get_root();
	//collasable 
	std::vector<scenenode::raw_pointer> parents;
	std::stack<scenenode::raw_pointer> s;
	scenenode::raw_pointer curr = root_node.get();
	s.push(curr);
	while (!s.empty())
	{
		s.pop();
		ImGuiTreeNodeFlags_ flags = ImGuiTreeNodeFlags_None;
		auto handle = curr->get_handle();
		for (auto selectedhandle : s_selected)
		{
			if (handle == selectedhandle)
				flags = static_cast<ImGuiTreeNodeFlags_>(flags | ImGuiTreeNodeFlags_Selected);
		}
		//pass this to scene to get game object
		if (curr->get_direct_child_count())//tree push
		{
			flags = static_cast<ImGuiTreeNodeFlags_>(flags | ImGuiTreeNodeFlags_OpenOnArrow);
		}
		else
			flags = static_cast<ImGuiTreeNodeFlags_>(flags | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);

		bool swapping = (curr->get_parent_handle() == m_dragged_parent) & m_isDragging;
		if (swapping && found_dragging == false)//first encounter
		{
			SwappingUI(*curr, found_dragging);
			swapping = true;
			found_dragging = true;
		}

		bool open = TreeNodeUI("object", *curr, flags, swapping);
		if (open == true && flags & ImGuiTreeNodeFlags_OpenOnArrow)
		{
			parents.push_back(curr);
		}

		//drag and drop option

		if (open && flags & ImGuiTreeNodeFlags_OpenOnArrow)
		{
			if (curr->get_direct_child_count())
			{
				for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
				{
					scenenode::shared_pointer child = *iter;
					s.push(child.get());
				}
			}
			else
			{
				auto parent_handle = s.top()->get_parent_handle();
				while (parents.empty() == false)
				{
					auto c_handle = parents.back()->get_handle();
					if (c_handle == parent_handle)
					{
						break;
					}
					else
					{
						ImGui::TreePop();
						parents.pop_back();
					}
				}
			}
		}
		if (s.empty())
		{
			while (parents.empty() == false)
			{
				ImGui::TreePop();
				parents.pop_back();
			}
			break;
		}
		curr = s.top();
	}

}

bool Hierarchy::TreeNodeUI(const char* name, scenenode& node, ImGuiTreeNodeFlags_ flags, bool swaping)
{
	auto handle = node.get_handle();
	ImGui::PushID(static_cast<int>(handle));
	bool open = (ImGui::TreeNodeEx(name, flags));
	ImGui::PopID();
	if (ImGui::IsItemHovered() &&
		(ImGui::IsMouseReleased(ImGuiMouseButton_Left)
			|| ImGui::IsMouseClicked(ImGuiMouseButton_Right)
			|| ImGui::IsKeyPressed(static_cast<int>(oo::input::KeyCode::ENTER))))
	{
		if (ImGui::IsKeyPressed(static_cast<int>(oo::input::KeyCode::LSHIFT)))
			s_selected.push_back(handle);
		else
		{
			s_selected.clear();
			s_selected.push_back(handle);
		}
	}

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_name);//just clear the payload from the eventsystem
		if (payload)
		{
			auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
			auto source = scene->FindWithInstanceID(m_dragged);
			auto targetparent = scene->FindWithInstanceID(node.get_handle());
			targetparent->AddChild(*source);//s_selected
		}
		ImGui::EndDragDropTarget();
	}
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload | ImGuiDragDropFlags_SourceAllowNullID))
	{
		ImGui::SetDragDropPayload(payload_name, nullptr, 0);
		m_isDragging = true;
		m_dragged = handle;
		m_dragged_parent = node.get_parent_handle();
		ImGui::Text("Dragging [%s]", "object");
		ImGui::EndDragDropSource();
	}
	if (swaping)
		SwappingUI(node);
	return open;
}

void Hierarchy::SwappingUI(scenenode& node, bool setbelow)
{

	ImGui::Separator();

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_name);//just clear the payload from the eventsystem
		if (payload)
		{
			if (setbelow)
			{
				//swap as younger sibling
				
			}
			else
			{
				//swap as oldest sibling(first object)
			}

		}
		ImGui::EndDragDropTarget();
	}
	return;
}

const std::vector<scenenode::handle_type>& Hierarchy::GetSelected()
{
	return s_selected;
}
void Hierarchy::SearchFilter()
{
	{//for drawing the search bar
		ImGui::PushItemWidth(-60.0f);
		ImVec2 cursor_pos = ImGui::GetCursorPos();
		ImGui::InputText("##Search", &m_filter, ImGuiInputTextFlags_EnterReturnsTrue);
		ImVec2 cursor_pos2 = ImGui::GetCursorPos();
		ImGui::SetCursorPos(cursor_pos);
		if (ImGui::IsItemActive() == false && m_filter.empty() == true)
			ImGui::Text("Search");
		ImGui::PopItemWidth();
		ImGui::SetCursorPos(cursor_pos2);
	}
	// can use color button here but extend it to have multiple selections
	// m_filterTypes = ColorButton();

}

void Hierarchy::Filter_ByName()
{
}

void Hierarchy::Filter_ByComponent()
{
}

void Hierarchy::Filter_ByScript()
{
}
