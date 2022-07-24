#pragma once
#include <vector>
#include <scenenode.h>
class Hierarchy
{
public:
	Hierarchy();
	void Show();
	static const std::vector<scenenode::handle_type>& GetSelected();
private:
	inline static std::vector<scenenode::handle_type> s_selected;

};
