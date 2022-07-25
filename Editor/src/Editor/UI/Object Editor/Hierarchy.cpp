#include <pch.h>
#include <Scenegraph.h>
#include <imgui.h>
//#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <stack>
#include <Ouroboros/Core/KeyCode.h>
#include "Hierarchy.h"
Hierarchy::Hierarchy()
{
}

void Hierarchy::Show()
{
	if(m_isDragging && !ImGui::IsMouseDragging(ImGuiMouseButton_::ImGuiMouseButton_Left))
		m_isDragging = false;//false if not dragging
	bool found_dragging = false;

	scenegraph instance{"name"};//the scene graph should be obtained instead.
	ImGui::BeginChild("search bar", {0,20},true);
	ImGui::EndChild();



	scenenode::shared_pointer root_node = instance.get_root();
	//collasable 

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
				flags = static_cast<ImGuiTreeNodeFlags_>(flags|ImGuiTreeNodeFlags_Selected);
		}
		//pass this to scene to get game object
		if (curr->get_direct_child_count())
			flags = static_cast<ImGuiTreeNodeFlags_>(flags | ImGuiTreeNodeFlags_OpenOnArrow);
		else
			flags = static_cast<ImGuiTreeNodeFlags_>(flags | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
		
		bool swapping = curr->get_parent_handle() == m_dragged_parent;
		if (swapping && found_dragging == false)//first encounter
		{
			SwappingUI(*curr, found_dragging);
			swapping = true;
			found_dragging = true;
		}
		bool open = TreeNodeUI("object",*curr, flags, swapping);

		
		//drag and drop option
		
		if (open && flags & ImGuiTreeNodeFlags_OpenOnArrow)
		{
			for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
			{
				scenenode::shared_pointer child = *iter;
				s.push(child.get());
			}
		}

		curr = s.top();
	}
	
}

bool Hierarchy::TreeNodeUI(const char* name,scenenode& node, ImGuiTreeNodeFlags_ flags, bool swaping)
{
	auto handle = node.get_handle();
	ImGui::PushID(handle);
	bool open = (ImGui::TreeNodeEx("object", flags));
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
			//node.add_child();//s_selected
		}
		ImGui::EndDragDropTarget();
	}
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_SourceAutoExpirePayload | ImGuiDragDropFlags_SourceAllowNullID))
	{
		ImGui::SetDragDropPayload(payload_name, nullptr, 0);
		m_isDragging = true;
		m_dragged = handle;
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
