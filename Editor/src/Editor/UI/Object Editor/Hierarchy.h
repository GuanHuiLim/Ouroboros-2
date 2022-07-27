#pragma once
#include <vector>
#include <scenegraph/include/scenenode.h>
#include <string>
enum ImGuiTreeNodeFlags_;//pre-declare
class Hierarchy
{
public:
	Hierarchy();
	void Show();
protected:
	bool TreeNodeUI(const char* name,scenenode& node, ImGuiTreeNodeFlags_ flag,bool swaping = false);
	void SwappingUI(scenenode& node,bool setbelow = true);
	void SearchFilter();
	static const std::vector<scenenode::handle_type>& GetSelected();
public:
	static constexpr const char* const payload_name = "HIERARCHY_PAYLOAD";
private:
	std::string m_filter;
	scenenode::handle_type m_dragged;
	scenenode::handle_type m_dragged_parent = 0;
	bool m_isDragging = false;
	//static
	inline static std::vector<scenenode::handle_type> s_selected;
};
