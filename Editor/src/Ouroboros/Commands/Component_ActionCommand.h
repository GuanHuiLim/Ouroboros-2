#pragma once
#include "ActionCommand.h"
#include <rttr/variant.h>
#include <rttr/property.h>
#include <SceneManagement/include/SceneManager.h>
#include <string>

#include "Ouroboros/ECS/GameObject.h"

#include "App/Editor/Utility/ImGuiManager.h"
#include "App/Editor/Networking/PacketUtils.h"
#include "App/Editor/Serializer.h"
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
	std::string GetData();
	Component_ActionCommand(PacketHeader& header,const std::string& data);
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
	PacketUtilts::BroadCastCommand(CommandPacketType::ActionObject, GetData());
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
	prop.set_value(comp, before);
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

template<typename Component>
inline std::string Component_ActionCommand<Component>::GetData()
{
	rttr::type t = rttr::type::get<Component>();
	std::string data = "";
	data += t.get_name().data();//this will be trimed out before reaching the Constructor
	data += PacketUtilts::SEPERATOR;
	data += std::to_string(gameobject_ID);
	data += PacketUtilts::SEPERATOR;
	data += Serializer::SaveSingleVariant(t, prop, before);
	data += PacketUtilts::SEPERATOR;
	data += Serializer::SaveSingleVariant(t, prop, after);
	return data;
}

template<typename Component>
inline Component_ActionCommand<Component>::Component_ActionCommand(PacketHeader & header,const std::string& data)
	:prop{*rttr::type::get<Component>().get_properties().begin() }
{
	size_t offset = 0;
	gameobject_ID = std::stoull(PacketUtilts::ParseCommandData(data, offset));
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto go = scene->FindWithInstanceID(gameobject_ID);
	if (go == nullptr)
		return;

	Serializer::LoadSingleVariant<Component>(*go, before, prop, PacketUtilts::ParseCommandData(data, offset));
	Serializer::LoadSingleVariant<Component>(*go, after, prop, std::string(data.begin() + (offset), data.end()));
	//once done run Redo() to apply the things
	Redo();
}

#include <unordered_map>
#include <functional>
//helper container 
//contains methods
class ActionCommandHelper
{
public:
	static void Init();
	static ActionCommand* CreateActionCommand(PacketHeader& header, const std::string& data);
private:
	inline static std::unordered_map<std::string, std::function<ActionCommand* (PacketHeader& ,const std::string&)>> s_createActionCommand;
	template <typename Component>
	static ActionCommand* CreateCommand(PacketHeader& header,const std::string& data)
	{
		return new Component_ActionCommand<Component>(header,data);
	}
	template <typename Component>
	static void AddCommandToMap()
	{
		s_createActionCommand.emplace(rttr::type::get<Component>().get_name().data(), &CreateCommand<Component>);
	}
};
}

