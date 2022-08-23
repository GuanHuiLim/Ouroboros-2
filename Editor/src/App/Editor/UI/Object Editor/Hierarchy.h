#pragma once
#include <vector>
#include <scenegraph/include/Scenegraph.h>
#include <string>
#include "App/Editor/Utility/ImGui_ToggleButton.h"
enum ImGuiTreeNodeFlags_;//pre-declare
class Hierarchy
{
public:
	Hierarchy();
	void Show();
	static const std::vector<scenenode::handle_type>& GetSelected();
protected:
	void NormalView();
	void FilteredView();


	bool TreeNodeUI(const char* name, scenenode& node, ImGuiTreeNodeFlags_ flag, bool swaping = false, bool rename = false);
	void SwappingUI(scenenode& node, bool setbelow = true);
	void SearchFilter();
	void RightClickOptions();

	void Filter_ByName();
	void Filter_ByComponent();
	void Filter_ByScript();

	void CreateGameObjectImmediate();
public:
	static constexpr const char* const payload_name = "HIERARCHY_PAYLOAD";
	static constexpr const unsigned int Popup_ID = 100000;
private:
	enum class FilterTypes
	{
		Name = 0,
		Component = 1,
		Scripts = 2,
	};
	ColorButton m_colorButton;
	std::vector<scenenode::handle_type> m_filterList;
	std::string m_curr_sceneFilepath = "";
	std::string m_filter = "";
	FilterTypes m_filterTypes = FilterTypes::Name;
	scenenode::handle_type m_hovered = (scenenode::handle_type)-100;
	scenenode::handle_type m_renaming = (scenenode::handle_type)-100;
	scenenode::handle_type m_dragged = (scenenode::handle_type)-100;
	scenenode::handle_type m_dragged_parent = (scenenode::handle_type)-100;
	bool m_isDragging = false;
	bool m_isRename = false;
	bool m_previewPrefab = false;
	//static
	inline static std::vector<scenenode::handle_type> s_selected;
};
