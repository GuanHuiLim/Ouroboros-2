#pragma once
#include <vector>
#include <scenegraph/include/Scenegraph.h>
#include <string>
enum ImGuiTreeNodeFlags_;//pre-declare
class Hierarchy
{
public:
	Hierarchy();
	void Show();
protected:
	bool TreeNodeUI(const char* name, scenenode& node, ImGuiTreeNodeFlags_ flag, bool swaping = false);
	void SwappingUI(scenenode& node, bool setbelow = true);
	void SearchFilter();

	void Filter_ByName();
	void Filter_ByComponent();
	void Filter_ByScript();

	static const std::vector<scenenode::handle_type>& GetSelected();

public:
	static constexpr const char* const payload_name = "HIERARCHY_PAYLOAD";
private:
	enum class FilterTypes
	{
		Name = 0,
		Component = 1,
		Scripts = 2,
	};
	std::string m_filter = "";
	FilterTypes m_filterTypes = FilterTypes::Name;
	scenenode::handle_type m_dragged = -100;
	scenenode::handle_type m_dragged_parent = -100;
	bool m_isDragging = false;
	//static
	inline static std::vector<scenenode::handle_type> s_selected;
};
