#pragma once
#include "ActionCommand.h"
#include "App/Editor/Networking/PacketUtils.h"

#include <unordered_map>
#include <functional>

//other helper commands
#include "Ouroboros/Commands/Component_ActionCommand.h"
#include "Ouroboros/Commands/AddComponent_ActionCommand.h"
//helper container 
//contains methods
namespace oo
{

class ActionCommandHelper
{
public:

	static void Init();
	static ActionCommand* CreateActionCommand(PacketHeader& header, const std::string& data);
	static ActionCommand* CreateActionCommand_Add(PacketHeader& header, const std::string& data);
	static ActionCommand* CreateActionCommand_Remove(PacketHeader& header, const std::string& data);
private:
	inline static std::unordered_map<std::string, std::function<ActionCommand* (PacketHeader&, const std::string&)>> s_createActionCommand;
	inline static std::unordered_map<std::string, std::function<ActionCommand* (PacketHeader&, const std::string&)>> s_createAddComponentCommand;
	inline static std::unordered_map<std::string, std::function<ActionCommand* (PacketHeader&, const std::string&)>> s_createRemoveComponentCommand;
	template <typename Component>
	static ActionCommand* CreateCommand_Action(PacketHeader& header, const std::string& data)
	{
		return new Component_ActionCommand<Component>(header, data);
	}
	template <typename Component>
	static ActionCommand* CreateCommand_Add(PacketHeader& header, const std::string& data)
	{
		return new AddComponent_ActionCommand<Component>(header, data);
	}
	template <typename Component>
	static ActionCommand* CreateCommand_Remove(PacketHeader& header, const std::string& data)
	{
		return new RemoveComponent_ActionCommand<Component>(header, data);
	}
	template <typename Component>
	static void AddCommandToMap()
	{
		s_createActionCommand.emplace(rttr::type::get<Component>().get_name().data(), &CreateCommand_Action<Component>);
		s_createAddComponentCommand.emplace(rttr::type::get<Component>().get_name().data(), &CreateCommand_Add<Component>);
		s_createRemoveComponentCommand.emplace(rttr::type::get<Component>().get_name().data(), &CreateCommand_Remove<Component>);
	}
};
}