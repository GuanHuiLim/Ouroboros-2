#include <pch.h>
#include <Scenegraph.h>
#include <imgui.h>
//#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <queue>
#include <Ouroboros/Core/KeyCode.h>
#include "Hierarchy.h"
Hierarchy::Hierarchy()
{
}

void Hierarchy::Show()
{
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
		ImGui::PushID(handle);
		//pass this to scene to get game object
		if (curr->get_direct_child_count())
			flags = static_cast<ImGuiTreeNodeFlags_>(flags | ImGuiTreeNodeFlags_OpenOnArrow);
		else
			flags = static_cast<ImGuiTreeNodeFlags_>(flags | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);

		bool open = (ImGui::TreeNodeEx("object", flags));
		if (ImGui::IsItemHovered()&&
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
		//drag and drop option
		ImGui::PopID();
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

const std::vector<scenenode::handle_type>& Hierarchy::GetSelected()
{
	return s_selected;
}
