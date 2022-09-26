/************************************************************************************//*!
\file          Hierarchy.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Allows the user to Edit the entity in scene.
			   Add object
			   Delete object
			   Make Prefab
			   Update Prefab
			   Open Prefab Editor

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include <pch.h>
#include <stack>
#include "Hierarchy.h"
#include "App/Editor/Serializer.h"
//utility
#include "App/Editor/Utility/ImGuiManager.h"
#include "App/Editor/Utility/ImGuiStylePresets.h"

//other UI functions
#include "App/Editor/UI/Tools/MeshHierarchy.h"


//imgui
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

//scene
#include <Ouroboros/Scene/Scene.h>
#include <Ouroboros/Prefab/PrefabComponent.h>
#include <Scenegraph/include/scenenode.h>
#include <Scenegraph/include/Scenegraph.h>
#include <SceneManagement/include/SceneManager.h>

//ouro utility
#include <Ouroboros/Core/KeyCode.h>
#include <Ouroboros/ECS/GameObject.h>

//events
#include <App/Editor/Events/OpenFileEvent.h>
#include <Ouroboros/EventSystem/EventManager.h>
#include <Ouroboros/Commands/CommandStackManager.h>
#include <Ouroboros/Commands/Delete_ActionCommand.h>

Hierarchy::Hierarchy()
	:m_colorButton({ "Name","Component","Scripts" }, 
		{ ImColor(0.75f,0.2f,0.3f),ImColor(0.3f,0.75f,0.2f),ImColor(0.2f,0.3f,0.75f) },
		ImVec2{0,0},0)
{
}

void Hierarchy::Show()
{
	ImGui::BeginChild("search bar", { 0,40 }, false);
	SearchFilter();
	ImGui::EndChild();

	m_filter.empty() ? NormalView() : FilteredView();
}

bool Hierarchy::TreeNodeUI(const char* name, scenenode& node, ImGuiTreeNodeFlags_ flags, bool swaping, bool rename,bool no_Interaction)
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
			targetparent->AddChild(*source, true);//s_selected
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
			if (ImGui::IsKeyDown(static_cast<int>(oo::input::KeyCode::LSHIFT)))
				s_selected.emplace(handle);
			else
			{
				s_selected.clear();
				s_selected.emplace(handle);
			}
		}
	}
	else if (clicked)
	{
		m_isRename = false;
	}

	if (no_Interaction)
		return open;

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload))
	{
		UUID goID = node.get_handle();
		ImGui::SetDragDropPayload(payload_name, &goID , sizeof(UUID));
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
	ImGui::PushID(static_cast<int>(node.get_handle()));
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

const std::set<scenenode::handle_type>& Hierarchy::GetSelected()
{
	return s_selected;
}
void Hierarchy::NormalView()
{
	RightClickOptions();

	{
		ImVec2 temp = ImGui::GetCursorPos();
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
		if (ImGui::Selectable("##parent to root", false, ImGuiSelectableFlags_AllowItemOverlap, ImGui::GetContentRegionAvail()))
			s_selected.clear();
		ImGui::SetCursorPos(temp);
		ImGui::PopStyleColor(2);
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_name);//just clear the payload from the eventsystem
			if (payload)
			{
				auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
				auto source = scene->FindWithInstanceID(m_dragged);
				scene->GetRoot()->AddChild(*source, true);//s_selected
			}
			payload = ImGui::AcceptDragDropPayload(".prefab"); //for creating prefab files
			if (payload)
			{
				auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
				auto go = scene->GetRoot();
				std::filesystem::path prefabpath = *static_cast<std::filesystem::path*>(payload->Data);
				Serializer::LoadPrefab(prefabpath,go,*scene);
			}
			payload = ImGui::AcceptDragDropPayload("MESH_HIERARCHY"); //for creating prefab files
			if (payload)
			{
				auto data = *static_cast<MeshHierarchy::MeshHierarchyDragDropData*>(payload->Data);
				MeshHierarchy::CreateObject(data.data,data.id);
			}
			ImGui::EndDragDropTarget();
		}
	}
	if (m_previewPrefab)
	{
		ImGui::Separator();
		if (ImGui::Button("Back"))
		{
			m_previewPrefab = false;
			OpenFileEvent ofe(m_curr_sceneFilepath);
			oo::EventManager::Broadcast(&ofe);
		}	
		ImGui::SameLine();
		ImGui::Text("Prefab Editing");
		ImGui::Separator();
	}

	//editor stuff
	bool found_dragging = false;
	bool rename_item = false;
	
	//scene stuff
	scenegraph instance = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->GetGraph();//the scene graph should be obtained instead.
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	scenenode::shared_pointer root_node = instance.get_root();

	//collasable 
	std::vector<scenenode::raw_pointer> parents;
	std::stack<scenenode::raw_pointer> s;
	scenenode::raw_pointer curr = root_node.get();

	//prefab stuff
	std::shared_ptr<oo::GameObject> prefabobj;
	bool open_prefab = false;

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

		bool open = false;
		if (source->GetIsPrefab())//prefab
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui_StylePresets::prefab_text_color);
			open = TreeNodeUI(name.c_str(), *curr, flags, swapping, rename_item, !source->HasComponent<oo::PrefabComponent>());
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				open_prefab = true;
				prefabobj = source;
			}
			ImGui::PopStyleColor();
		}
		else
			open = TreeNodeUI(name.c_str(), *curr, flags, swapping, rename_item);

		if (open == true && (flags & ImGuiTreeNodeFlags_OpenOnArrow))
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
		else if (s.empty() == false)
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
	if (open_prefab)
	{
		m_previewPrefab = true;
		m_curr_sceneFilepath = scene->GetFilePath();
		auto complete_path = Project::GetPrefabFolder() / prefabobj->GetComponent<oo::PrefabComponent>().prefab_filePath;
		OpenFileEvent ofe(complete_path);
		oo::EventManager::Broadcast(&ofe);
	}
}
void Hierarchy::FilteredView()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	ImGui::Text("Filtered Search");
	ImGui::Separator();
	for (auto handle : m_filterList)
	{
		auto go = scene->FindWithInstanceID(handle);
		bool selected = false;
		for (auto selectedhandle : s_selected)
		{
			if (handle == selectedhandle)
			{
				selected = true;
				break;
			}
		}
		ImGui::PushID(static_cast<int>(handle));
		ImGui::Selectable(go->Name().c_str(),selected);
		ImGui::PopID();

		bool hovered = ImGui::IsItemHovered();
		bool clicked = ImGui::IsMouseReleased(ImGuiMouseButton_Left) | ImGui::IsMouseClicked(ImGuiMouseButton_Right);
		bool keyenter = ImGui::IsKeyPressed(static_cast<int>(oo::input::KeyCode::ENTER));
		if (hovered)
		{
			m_hovered = handle;
			if ((clicked || keyenter))
			{
				if (ImGui::IsKeyPressed(static_cast<int>(oo::input::KeyCode::LSHIFT)))
					s_selected.emplace(handle);
				else
				{
					s_selected.clear();
					s_selected.emplace(handle);
				}
			}
		}
	}
}
void Hierarchy::SearchFilter()
{
	{//for drawing the search bar
		ImVec2 cursor_pos = ImGui::GetCursorPos();
		ImGui::PushItemWidth(-100.0f);
		bool edited = ImGui::InputText("##Search", &m_filter);
		ImGui::PopItemWidth();
		ImVec2 cursor_pos2 = ImGui::GetCursorPos();
		ImGui::SameLine(cursor_pos2.x);
		if (ImGui::Button("Clear"))
			m_filter.clear();

		ImGui::SameLine();
		m_colorButton.UpdateToggle();

		

		ImGui::SetCursorPos(cursor_pos);
		if (ImGui::IsItemActive() == false && m_filter.empty() == true)
			ImGui::Text("Search");
		//ImGui::SetCursorPos(cursor_pos2);


		if(edited)
		switch (m_colorButton.GetIndex())
		{
		case 0:
			Filter_ByName();
			break;
		case 1:
			break;
		case 2:
			break;
		}

	}
}

void Hierarchy::RightClickOptions()
{
	if (ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
		ImGui::OpenPopupEx(Popup_ID);
	if (ImGui::BeginPopupEx(Popup_ID, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
	{
		
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("New GameObject"))
			{
				CreateGameObjectImmediate();
			}
			if(ImGui::MenuItem("Box"))
			{
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Destroy GameObject"))
		{
			auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
			for (auto go : s_selected)
			{
				auto object = scene->FindWithInstanceID(go);
				if (object->HasComponent<oo::PrefabComponent>() == false && object->GetIsPrefab())
					continue;
				oo::CommandStackManager::AddCommand(new oo::Delete_ActionCommand(object));
				object->Destroy();
			}
			s_selected.clear();
		}
		if (ImGui::MenuItem("Duplicate GameObject"))
		{
			auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
			for (auto go : s_selected)
			{
				auto object = scene->FindWithInstanceID(go);
				if (object->HasComponent<oo::PrefabComponent>() == false && object->GetIsPrefab())
					continue;
				object->Duplicate();
			}
		}
		ImGui::EndPopup();
	}

}

void Hierarchy::Filter_ByName()
{
	m_filterList.clear();
	scenegraph instance = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->GetGraph();
	auto handles = instance.hierarchy_traversal_handles();
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	for (auto curr_handle : handles)
	{
		std::shared_ptr<oo::GameObject> go = scene->FindWithInstanceID(curr_handle);
		std::string& name = go->Name();
		
		auto iter = std::search(name.begin(), name.end(),
			m_filter.begin(), m_filter.end(),
			[](char ch1, char ch2) 
			{
				return std::toupper(ch1) == std::toupper(ch2); 
			});
		if (iter != name.end())
			m_filterList.emplace_back(curr_handle);
	}
}

void Hierarchy::Filter_ByComponent()
{
}

void Hierarchy::Filter_ByScript()
{
}

void Hierarchy::CreateGameObjectImmediate()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto go = scene->CreateGameObjectImmediate();
	go->SetName("New GameObject");
	if (s_selected.size() == 1 && m_hovered == *(s_selected.begin()))
	{
		auto parent_object = scene->FindWithInstanceID(m_hovered);
		if (parent_object->GetIsPrefab())
			return;
		parent_object->AddChild(*go);
	}
}
