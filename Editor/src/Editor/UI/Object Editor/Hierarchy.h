#pragma once
#include <vector>
#include <scenenode.h>
enum ImGuiTreeNodeFlags_;//pre-declare
class Hierarchy
{
public:
	Hierarchy();
	void Show();
protected:
	bool TreeNodeUI(const char* name,scenenode& node, ImGuiTreeNodeFlags_ flag,bool swaping = false);
	void SwappingUI(scenenode& node,bool setbelow = true);
	static const std::vector<scenenode::handle_type>& GetSelected();
public:
	static constexpr const char* const payload_name = "HIERARCHY_PAYLOAD";
private:
	bool m_isDragging = false;
	scenenode::handle_type m_dragged;
	scenenode::handle_type m_dragged_parent = 0;
	inline static std::vector<scenenode::handle_type> s_selected;

};
