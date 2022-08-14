#include <pch.h>
#include "Hierarchy.h"
#include "App/Editor/Utility/ImGuiManager.h"

#include <stack>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <Ouroboros/Core/KeyCode.h>
#include <Ouroboros/ECS/GameObject.h>
#include <Scenegraph/include/scenenode.h>
#include <Scenegraph/include/Scenegraph.h>
#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/Scene/Scene.h>

Hierarchy::Hierarchy()
	:m_colorButton({ "Name","Component","Scripts" }, 
		{ ImColor(0.75f,0.2f,0.3f),ImColor(0.3f,0.75f,0.2f),ImColor(0.2f,0.3f,0.75f) },
		ImVec2{0,0},0)
{
}

void Hierarchy::Show()
{
	bool found_dragging = false;
	bool rename_item = false;
	scenegraph instance = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->GetGraph();//the scene graph should be obtained instead.
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	
	ImGui::BeginChild("search bar", { 0,40 }, false);
	SearchFilter();
	ImGui::EndChild();

	scenenode::shared_pointer root_node = instance.get_root();
	//collasable 
	std::vector<scenenode::raw_pointer> parents;
	std::stack<scenenode::raw_pointer> s;
	scenenode::raw_pointer curr = root_node.get();

	for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
	{
		scenenode::shared_pointer child = *iter;
		s.push(child.get());
	}

	while (!s.empty())
	{
		curr = s.top();
		s.pop();
		ImGuiTreeNodeFlags_ flags = ImGuiTreeNodeFlags_None;
		auto handle = curr->get_handle();
		rename_item = false;
		for (auto selectedhandle : s_selected)
		{
			if (handle == selectedhandle)
			{
				flags = static_cast<ImGuiTreeNodeFlags_>(flags | ImGuiTreeNodeFlags_Selected);
				break;
			}
		}
		if (m_isRename && m_renaming == handle)
		{
			rename_item = true;
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
		auto source = scene->FindWithInstanceID(curr->get_handle());
		std::string name = "";
		if (source)
			name = source->Name();
		bool open = TreeNodeUI(name.c_str(), *curr, flags, swapping,rename_item);
		if (open == true && flags & ImGuiTreeNodeFlags_OpenOnArrow)
		{
			parents.push_back(curr);
		}

		//drag and drop option


		if (open && flags & ImGuiTreeNodeFlags_OpenOnArrow && curr->get_direct_child_count())
		{
			for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
			{
				scenenode::shared_pointer child = *iter;
				s.push(child.get());
			}
		}
		else if(s.empty() == false)
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
		if (s.empty())
		{
			while (parents.empty() == false)
			{
				ImGui::TreePop();
				parents.pop_back();
			}
			break;
		}
	}
	if (m_isDragging && !ImGui::IsMouseDragging(ImGuiMouseButton_::ImGuiMouseButton_Left))
		m_isDragging = false;//false if not dragging
	if (ImGui::IsKeyDown(ImGuiKey_F2))
	{
		m_isRename = true;
		m_renaming = m_hovered;
	}
}

bool Hierarchy::TreeNodeUI(const char* name, scenenode& node, ImGuiTreeNodeFlags_ flags, bool swaping, bool rename)
{
	auto handle = node.get_handle();
	ImGui::PushID(static_cast<int>(handle));
	bool open = false;
	if (!rename)
		open = (ImGui::TreeNodeEx(name, flags));
	else
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto source = scene->FindWithInstanceID(node.get_handle());
		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("##rename", &source->Name(), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
			m_isRename = false;
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
	ImGui::PopID();
	bool hovered = ImGui::IsItemHovered();
	bool clicked = ImGui::IsMouseReleased(ImGuiMouseButton_Left) | ImGui::IsMouseClicked(ImGuiMouseButton_Right);
	bool keyenter = ImGui::IsKeyPressed(static_cast<int>(oo::input::KeyCode::ENTER));
	if (hovered)
	{
		m_hovered = node.get_handle();
		if ((clicked || keyenter))
		{
			if (ImGui::IsKeyPressed(static_cast<int>(oo::input::KeyCode::LSHIFT)))
				s_selected.push_back(handle);
			else
			{
				s_selected.clear();
				s_selected.push_back(handle);
			}
		}
	}
	else if (clicked)
	{
		m_isRename = false;
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload))
	{
		ImGui::SetDragDropPayload(payload_name, nullptr, 0);
		m_isDragging = true;
		m_dragged = handle;
		m_dragged_parent = node.get_parent_handle();
		ImGui::Text("Dragging [%s]", "object");
		ImGui::EndDragDropSource();
	}

	if (swaping)
		SwappingUI(node,true);
	return open;
}

void Hierarchy::SwappingUI(scenenode& node, bool setbelow)
{
	ImGui::PushID(node.get_handle());
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_ItemSpacing, { 0,1.0f });
	ImVec2 pos = ImGui::GetCursorPos();
	ImGui::Selectable("--------", false, ImGuiSelectableFlags_::ImGuiSelectableFlags_None, {0,8.0f});
	ImGui::PopStyleVar();

	if (ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()))
	{
		ImGui::SetCursorPos(pos);
		ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_ItemSpacing, { 0,5.0f });
		ImGui::Selectable("----------------------", false, ImGuiSelectableFlags_::ImGuiSelectableFlags_Disabled, { 0,10.0f });
		ImGui::PopStyleVar();
	}
	ImGui::PopID();
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_name);//just clear the payload from the eventsystem
		if (payload)
		{
			if (setbelow)
			{
				//swap as younger sibling
				auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
				auto source = scene->FindWithInstanceID(m_dragged);
				source->GetSceneNode().lock()->move_to_after(node.shared_from_this());
			}
			else
			{
				//swap as oldest sibling(first object)
				auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
				auto source = scene->FindWithInstanceID(m_dragged);
				source->GetSceneNode().lock()->move_to(node.shared_from_this());
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
		ImGui::PushItemWidth(-100.0f);
		ImVec2 cursor_pos = ImGui::GetCursorPos();
		ImGui::InputText("##Search", &m_filter, ImGuiInputTextFlags_EnterReturnsTrue);
		ImVec2 cursor_pos2 = ImGui::GetCursorPos();
		ImGui::SetCursorPos(cursor_pos);
		if (ImGui::IsItemActive() == false && m_filter.empty() == true)
			ImGui::Text("Search");
		ImGui::PopItemWidth();
		ImGui::SetCursorPos(cursor_pos2);
		ImGui::SameLine();
		m_colorButton.UpdateToggle();
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
