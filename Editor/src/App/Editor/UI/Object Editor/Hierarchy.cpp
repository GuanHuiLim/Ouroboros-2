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

//action command
#include <Ouroboros/Commands/CommandStackManager.h>
#include <Ouroboros/Commands/Delete_ActionCommand.h>
#include <Ouroboros/Commands/Ordering_ActionCommand.h>
#include <Ouroboros/Commands/Component_ActionCommand.h>
//events
#include <App/Editor/Events/OpenPromtEvent.h>
#include <App/Editor/Events/OpenFileEvent.h>
#include <Ouroboros/EventSystem/EventManager.h>
//networking
#include <App/Editor/Networking/NetworkingEvent.h>
#include <App/Editor/Networking/PacketUtils.h>
//profiller
#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>
//other components
#include <Ouroboros/Physics/RigidbodyComponent.h>
#include <Ouroboros/Physics/ColliderComponents.h>
//#include <Ouroboros/Vulkan/RendererComponent.h>
#include <Ouroboros/Vulkan/LightComponent.h>
#include <Ouroboros/Vulkan/MeshRendererComponent.h>
#include <Ouroboros/Vulkan/CameraComponent.h>

#include <Ouroboros/UI/GraphicsRaycasterComponent.h>
#include <Ouroboros/UI/RectTransformComponent.h>
#include <Ouroboros/UI/UICanvasComponent.h>
#include <Ouroboros/UI/UIImageComponent.h>
#include <Ouroboros/UI/UIRaycastComponent.h>

Hierarchy::Hierarchy()
	:m_colorButton({ "Name","Component","Scripts" }, 
		{ ImColor(0.75f,0.2f,0.3f),ImColor(0.3f,0.75f,0.2f),ImColor(0.2f,0.3f,0.75f) },
		ImVec2{0,0},0)
{
	oo::EventManager::Subscribe<CopyButtonEvent>(&CopyEvent);
	oo::EventManager::Subscribe<PasteButtonEvent>(&PasteEvent);
	oo::EventManager::Subscribe<NetworkingSelectionEvent>([] (NetworkingSelectionEvent* e){
		auto iter = Hierarchy::GetSelected().find(e->gameobjID);
		if (iter != s_selected.end())
		{
			if (Hierarchy::GetSelectedTime() > e->time_triggered)
				Hierarchy::GetSelectedNonConst().erase(iter);
			else
				return;
		}
		s_networkUserSelection[e->header.name] = ItemSelectedTiming{ e->time_triggered,e->gameobjID };
		});
	oo::EventManager::Subscribe<DuplicateButtonEvent>(&DuplicateEvent);
	oo::EventManager::Subscribe<DestroyGameObjectButtonEvent>(&DestroyEvent);

}

void Hierarchy::Show()
{
	
	TRACY_PROFILE_SCOPE_NC(hierarchy_ui_update, tracy::Color::BlueViolet);
	
	ImGui::BeginChild("search bar", { 0,40 }, false);
	SearchFilter();
	ImGui::EndChild();

	m_filter.empty() ? NormalView() : FilteredView();

	TRACY_PROFILE_SCOPE_END();
}
/**
 * \param name - name of current node
 * \param node - current node
 * \param flags - treenodeflags
 * \param swaping - is item in swapping mode
 * \param rename - is item in rename mode
 * \param no_Interaction - true if there is interaction , false if no interaction
 * \return 
 */
bool Hierarchy::TreeNodeUI(const char* name, scenenode& node, ImGuiTreeNodeFlags_ flags, bool swaping, bool rename,bool no_Interaction)
{
	auto handle = node.get_handle();
	//networking code////////////
		bool networking_selected = false;
		for (auto& p : s_networkUserSelection)
		{
			if (p.second.gameobjecID == handle)
			{
				networking_selected = true;
				flags = (ImGuiTreeNodeFlags_) (flags |ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected);
				break;
			}
		}
	//end of networking code/////
	ImGui::PushID(static_cast<int>(handle));
	bool open = false;
	if (!rename)
	{
		if(networking_selected == false)
			open = (ImGui::TreeNodeEx(name, flags));
		else
		{
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Header, ImVec4(0.3f, 0.8f, 0.1f, 0.8f));
			open = (ImGui::TreeNodeEx(name, flags));
			ImGui::PopStyleColor();
		}
	}
	else
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto source = scene->FindWithInstanceID(node.get_handle());
		ImGui::SetKeyboardFocusHere();
		std::string curr_name = source->Name();
		if (ImGui::InputText("##rename", &curr_name, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
		{
			oo::CommandStackManager::AddCommand(new oo::Component_ActionCommand<oo::GameObjectComponent>
				(source->Name(), curr_name, rttr::type::get<oo::GameObjectComponent>().get_property("Name"), source->GetInstanceID()));
			source->Name() = curr_name;
			m_isRename = false;
		}
	}
	//nothing should parent over it if no_interaction == false
	if (ImGui::BeginDragDropTarget() && no_Interaction == false)
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_name);//just clear the payload from the eventsystem
		if (payload)
		{
			auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
			auto source = scene->FindWithInstanceID(m_dragged);
			auto targetparent = scene->FindWithInstanceID(node.get_handle());
			//action command before addchild
			oo::CommandStackManager::AddCommand(new oo::Parenting_ActionCommand(source, targetparent->GetInstanceID()));
			targetparent->AddChild(*source, true);//s_selected

		}
		ImGui::EndDragDropTarget();
	}
	ImGui::PopID();
	bool hovered = ImGui::IsItemHovered();
	bool clicked = ImGui::IsMouseReleased(ImGuiMouseButton_Left) | ImGui::IsMouseClicked(ImGuiMouseButton_Right);
	if (hovered)
	{
		bool mousedown = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
		bool keyenter = ImGui::IsKeyPressed(static_cast<int>(oo::input::KeyCode::ENTER));

		m_hovered = node.get_handle();
		if ((clicked || keyenter || (mousedown /*&& !clicked*/)) && s_selected.contains(handle) == false) //always insert values that unique (avoid unnesscary checks)
		{
			for (auto other_handles : s_networkUserSelection)
			{
				if (other_handles.second.gameobjecID == handle)
				{
					WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG,
						other_handles.first + " had this selected!");
					return open;
				}
			}
			if ((mousedown && !clicked) || ImGui::IsKeyDown(static_cast<int>(oo::input::KeyCode::LCTRL)))
				s_selected.emplace(handle);//this is hard to broadcast
			else
			{
				using namespace std::chrono;
				s_selected.clear();
				s_selected.emplace(handle);
				s_selectedTime_Epoc = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
				BroadcastSelection(handle);//networking part
			}
		}
	}
	else if (clicked)
	{
		m_isRename = false;
	}

	//if (no_Interaction)
	//	return open;

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload))
	{
		oo::UUID goID = node.get_handle();
		ImGui::SetDragDropPayload(payload_name, &goID , sizeof(oo::UUID));
		m_isDragging = true;
		m_dragged = handle;
		m_dragged_parent = node.get_parent_handle();
		ImGui::Text("Dragging [%s]", "object");
		ImGui::EndDragDropSource();
	}

	if (swaping && no_Interaction == false)
		SwappingUI(node,true);
	return open;
}

void Hierarchy::SwappingUI(scenenode& node, bool setbelow)
{
	if (node.get_handle() == m_dragged)
		return;
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
				oo::CommandStackManager::AddCommand(new oo::Ordering_ActionCommand(source, node.get_handle(), true));
				source->GetSceneNode().lock()->move_to_after(node.shared_from_this());
			}
			else
			{
				//swap as oldest sibling(first object)
				auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
				auto source = scene->FindWithInstanceID(m_dragged);
				oo::CommandStackManager::AddCommand(new oo::Ordering_ActionCommand(source, node.get_handle(), false));
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
std::set<scenenode::handle_type>& Hierarchy::GetSelectedNonConst()
{
	return s_selected;
}
void Hierarchy::SetItemSelected(scenenode::handle_type id)
{
	s_selected.clear();
	s_selected.emplace(id);
	BroadcastSelection(id);
}
const uint64_t Hierarchy::GetSelectedTime()
{
	return s_selectedTime_Epoc;
}
void Hierarchy::PreviewPrefab(const std::filesystem::path& p, const std::filesystem::path& currscene)
{
	PrefabSceneData data;
	data.m_curr_sceneFilepath = currscene.string();
	m_prefabsceneList.push_back(std::move(data));
	OpenPromptEvent<OpenFileEvent> ope(OpenFileEvent(p), 0);
	oo::EventManager::Broadcast(&ope);
}
void Hierarchy::PopBackPrefabStack()
{
	OpenPromptEvent<OpenFileEvent> ope(OpenFileEvent(m_prefabsceneList.back().m_curr_sceneFilepath), [this] {m_prefabsceneList.pop_back(); });
	oo::EventManager::Broadcast(&ope);
}
void Hierarchy::NormalView()
{
	RightClickOptions();

	{
		ImVec2 temp = ImGui::GetCursorPos();
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
		if (ImGui::Selectable("##parent to root", false, ImGuiSelectableFlags_AllowItemOverlap, ImGui::GetContentRegionAvail()))
		{
			s_selected.clear();
			BroadcastSelection(oo::UUID::Invalid);
		}
		ImGui::SetCursorPos(temp);
		ImGui::PopStyleColor(2);
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_name);//just clear the payload from the eventsystem
			if (payload)
			{
				auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
				auto source = scene->FindWithInstanceID(m_dragged);

				if (source->HasComponent<oo::PrefabComponent>() == false && source->GetIsPrefab())
				{
					WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, "Break Prefab if you want to unparent childs");
				}
				else
				{
					if (scene->GetRoot()->GetInstanceID() != source->GetParent().GetInstanceID())
					{
						oo::CommandStackManager::AddCommand(new oo::Parenting_ActionCommand(source, scene->GetRoot()->GetInstanceID()));
						scene->GetRoot()->AddChild(*source, true);//s_selected
					}
				}
			}
			payload = ImGui::AcceptDragDropPayload(".prefab"); //for creating prefab files
			if (payload)
			{
				auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
				auto go = scene->GetRoot();
				std::filesystem::path prefabpath = *static_cast<std::filesystem::path*>(payload->Data);
				oo::UUID prefab_obj = Serializer::LoadPrefab(prefabpath,go,*scene);
				oo::CommandStackManager::AddCommand(new oo::Create_ActionCommand(scene->FindWithInstanceID(prefab_obj)));
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
	//if empty then it shouldn't have been from a prefab scene
	if (m_prefabsceneList.empty() == false)
	{
		ImGui::Separator();
		if (ImGui::Button("Back"))
		{
			PopBackPrefabStack();
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

		auto source = scene->FindWithInstanceID(curr->get_handle());
		bool swapping = ((curr->get_parent_handle() == m_dragged_parent) & m_isDragging) ;
		if (swapping && found_dragging == false && source->GetIsPrefab() == false)//first encounter
		{
			SwappingUI(*curr, found_dragging);
			swapping = true;
			found_dragging = true;
		}
		std::string name = "";

		if (source)
			name = source->Name();

		bool open = false;
		if (source->GetIsPrefab())//prefab
		{

			ImGui::PushStyleColor(ImGuiCol_Text, ImGui_StylePresets::prefab_text_color);
			open = TreeNodeUI(name.c_str(), *curr, flags, swapping, rename_item, !source->HasComponent<oo::PrefabComponent>());
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && source->HasComponent<oo::PrefabComponent>())
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
		auto complete_path = Project::GetPrefabFolder() / prefabobj->GetComponent<oo::PrefabComponent>().prefab_filePath;
		PreviewPrefab(complete_path,scene->GetFilePath());
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
		if (hovered)
		{
			bool keyenter = ImGui::IsKeyPressed(static_cast<int>(oo::input::KeyCode::ENTER));
			bool mousedown = ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsMouseClicked(ImGuiMouseButton_Left);
			m_hovered = handle;
			if (( clicked || (mousedown /*&& !clicked*/) || keyenter) && s_selected.contains(handle) == false) //avoid unnessary checks
			{
				for (auto other_handles : s_networkUserSelection)
				{
					if (other_handles.second.gameobjecID == handle)
					{
						WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG,
							other_handles.first + " had this selected!");
						return;
					}
				}

				if ((mousedown && !clicked) || ImGui::IsKeyPressed(static_cast<int>(oo::input::KeyCode::LCTRL)))
					s_selected.emplace(handle);
				else
				{
					s_selected.clear();
					s_selected.emplace(handle);
					using namespace std::chrono;
					s_selectedTime_Epoc = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
					BroadcastSelection(handle);//networking part
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
				CreateGameObjectImmediate([](oo::GameObject& go) 
					{
						go.SetName("Box");
						go.EnsureComponent<oo::MeshRendererComponent>();
						go.EnsureComponent<oo::BoxColliderComponent>();
					});
			
			}
			if (ImGui::MenuItem("Light"))
			{
				CreateGameObjectImmediate([](oo::GameObject& go) 
					{
						go.SetName("Light");
						go.EnsureComponent<oo::LightComponent>();
					});

			}
			if (ImGui::MenuItem("Camera"))
			{
				CreateGameObjectImmediate([](oo::GameObject& go)
					{
						go.SetName("Camera");
						go.EnsureComponent<oo::CameraComponent>();
						// for now lets add a mesh to let us know where our camera is
						go.EnsureComponent<oo::MeshRendererComponent>();
					});
			}
			if (ImGui::MenuItem("UI Canvas"))
			{
				CreateGameObjectImmediate([](oo::GameObject& go)
					{
						go.SetName("Canvas");
						go.EnsureComponent<oo::RectTransformComponent>();
						go.EnsureComponent<oo::UICanvasComponent>();
						go.EnsureComponent<oo::GraphicsRaycasterComponent>();
					});
			}
			if (ImGui::MenuItem("UI Image"))
			{
				CreateGameObjectImmediate([](oo::GameObject& go)
					{
						go.SetName("UI Image");
						go.EnsureComponent<oo::RectTransformComponent>();
						go.EnsureComponent<oo::UICanvasComponent>();
						go.EnsureComponent<oo::UIRaycastComponent>();
						go.EnsureComponent<oo::UIImageComponent>();
					});
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
			BroadcastSelection(oo::UUID::Invalid);
		}
		if (ImGui::MenuItem("Duplicate GameObject"))
		{
			auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
			decltype(s_selected) created_list;
			for (auto go : s_selected)
			{
				auto object = scene->FindWithInstanceID(go);
				if (object->HasComponent<oo::PrefabComponent>() == false && object->GetIsPrefab())
					continue;
				oo::GameObject new_object = object->Duplicate();
				created_list.emplace(new_object.GetInstanceID());
				//need the scene::go_ptr
				auto goptr = scene->FindWithInstanceID(new_object.GetInstanceID());
				oo::CommandStackManager::AddCommand(new oo::Create_ActionCommand(goptr));
				if (object->HasValidParent())
				{
					oo::GameObject object_parent = object->GetParent();
					oo::CommandStackManager::AddCommand(new oo::Parenting_ActionCommand(goptr,object_parent.GetInstanceID()));
					object_parent.AddChild(new_object);
				}
			}
			s_selected.clear();
			s_selected = created_list;
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

std::shared_ptr<oo::GameObject> Hierarchy::CreateGameObjectImmediate(std::function<void(oo::GameObject&)> modifications)
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto go = scene->CreateGameObjectImmediate();
	go->SetName("New GameObject");
	if (modifications)
		modifications(*go);//for different types of presets

	oo::CommandStackManager::AddCommand(new oo::Create_ActionCommand(go));
	if (s_selected.size() == 1 && m_hovered == *(s_selected.begin()))
	{
		auto parent_object = scene->FindWithInstanceID(m_hovered);
		if (parent_object == nullptr)
			return go;
		if (parent_object->GetIsPrefab())
			return go;
		oo::CommandStackManager::AddCommand(new oo::Parenting_ActionCommand(go, parent_object->GetInstanceID()));
		parent_object->AddChild(*go);
		//parent item undo redo
	}
	return go;
}

void Hierarchy::CopyEvent(CopyButtonEvent* cbe)
{
	ImRect rect = ImGui::FindWindowByName("Hierarchy")->InnerRect;
	if (rect.Contains(ImGui::GetMousePos()) == false)
		return;
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto list = Hierarchy::GetSelected();
	if (list.empty() == false)
	{
		std::vector<oo::Scene::go_ptr> init_list;
		for (auto id : list)
		{
			init_list.push_back(scene->FindWithInstanceID(id));
		}
		Hierarchy::s_clipboard = Serializer::SaveObjectsAsString(init_list, *scene).c_str();
		ImGui::SetClipboardText(Hierarchy::s_clipboard.c_str());
	}
}

void Hierarchy::PasteEvent(PasteButtonEvent* pbe)
{
	ImRect rect = ImGui::FindWindowByName("Hierarchy")->InnerRect;
	if (rect.Contains(ImGui::GetMousePos()) == false)
		return;

	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	std::string data = ImGui::GetClipboardText();
	if (data.empty() == false)
	{
		auto created_items = Serializer::LoadObjectsFromString(Hierarchy::s_clipboard, scene->GetRoot()->GetInstanceID(), *scene);
		for (oo::UUID created_go : created_items)
		{
			auto created_go_ptr = scene->FindWithInstanceID(created_go);
			oo::CommandStackManager::AddCommand(new oo::Create_ActionCommand(created_go_ptr));
		}
	}

}

void Hierarchy::DuplicateEvent(DuplicateButtonEvent* dbe)
{
	ImRect rect = ImGui::FindWindowByName("Hierarchy")->InnerRect;
	if (rect.Contains(ImGui::GetMousePos()) == false)
		return;

	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	for (auto go : s_selected)
	{
		auto object = scene->FindWithInstanceID(go);
		if (object->HasComponent<oo::PrefabComponent>() == false && object->GetIsPrefab())
			continue;
		oo::GameObject new_object = object->Duplicate();
		//need the scene::go_ptr
		auto goptr = scene->FindWithInstanceID(new_object.GetInstanceID());
		oo::CommandStackManager::AddCommand(new oo::Create_ActionCommand(goptr));
	}
}

void Hierarchy::DestroyEvent(DestroyGameObjectButtonEvent* dbe)
{
	ImRect rect = ImGui::FindWindowByName("Hierarchy")->InnerRect;
	if (rect.Contains(ImGui::GetMousePos()) == false)
		return;

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

void Hierarchy::BroadcastSelection(oo::UUID gameobj)
{
	if (PacketUtilts::is_connected == false)
		return;
	using namespace std::chrono;
	uint64_t currTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	std::string data;
	data += std::to_string(currTime);
	data += PacketUtilts::SEPERATOR;
	data += std::to_string(gameobj);
	PacketUtilts::BroadCastCommand(CommandPacketType::Selected_Object, data);
}
