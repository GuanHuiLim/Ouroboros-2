#pragma once
#include "ActionCommand.h"
#include <rttr/variant.h>
#include <rttr/property.h>
#include <string>
#include "Ouroboros/ECS/GameObject.h"
#include "App/Editor/Utility/ImGuiManager.h"
namespace oo
{
template <typename Component>
class Component_ActionCommand :public ActionCommand
{
public:
	Component_ActionCommand(rttr::variant pre, rttr::variant post, rttr::property p, scenenode::handle_type id);
	~Component_ActionCommand();
	void Undo() override;
	void Redo() override;
	std::string ToString() override;
private:
	rttr::variant before;
	rttr::variant after;
	rttr::property prop;
	scenenode::handle_type gameobject_ID;
};

template<typename Component>
inline Component_ActionCommand<Component>::Component_ActionCommand(rttr::variant pre , rttr::variant post, rttr::property p , scenenode::handle_type id)
	:before{pre},
	after{post},
	prop{p},
	gameobject_ID{id}
{
	message = "Component Value Edited :";
}

template<typename Component>
inline Component_ActionCommand<Component>::~Component_ActionCommand()
{
	before.clear();//remove memory from variant
	after.clear();
	message.clear();
}

template<typename Component>
inline void Component_ActionCommand<Component>::Undo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto go = scene->FindWithInstanceID(gameobject_ID);
	if (go == nullptr)
		return;
	if (go->HasComponent<Component>() == false)
		return;
	auto& comp = go->GetComponent<Component>();
	bool valid = prop.set_value(comp, before);
}

template<typename Component>
inline void Component_ActionCommand<Component>::Redo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto go = scene->FindWithInstanceID(gameobject_ID);
	if (go == nullptr)
		return;
	if (go->HasComponent<Component>() == false)
		return;
	auto& comp = go->GetComponent<Component>();
	prop.set_value(comp, after);
}

template<typename Component>
inline std::string Component_ActionCommand<Component>::ToString()
{
	return message + prop.get_name();
}

}

