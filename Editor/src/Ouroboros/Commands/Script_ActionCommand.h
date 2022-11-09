#pragma once
#include "Ouroboros/Scripting/ScriptInfo.h"
#include "ActionCommand.h"
#include <string>
#include "App/Editor/Networking/PacketUtils.h"
namespace oo
{

class Script_ActionCommand :public ActionCommand
{
public:
	Script_ActionCommand(const std::string& scriptInfo, const std::string& scriptFieldInfo, oo::ScriptValue pre_value, oo::ScriptValue post_value, oo::UUID gameobjID);
	~Script_ActionCommand();
	void Undo() override;
	void Redo() override;
	std::string GetData();
	Script_ActionCommand(PacketHeader& packet, std::string& data);
private:
	std::string SI_name;
	std::string SFI_name;
	oo::UUID gameobjectID;

	oo::ScriptValue pre_val;
	oo::ScriptValue post_val;
};

}