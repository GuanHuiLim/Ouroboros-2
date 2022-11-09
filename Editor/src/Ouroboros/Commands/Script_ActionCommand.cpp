#include "pch.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"

#include "Script_ActionCommand.h"
#include "App/Editor/UI/Tools/WarningMessage.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include <SceneManagement/include/SceneManager.h>

#include "Ouroboros/Scripting/ScriptComponent.h"
#include "App/Editor/Serializer.h"
namespace oo
{
	Script_ActionCommand::Script_ActionCommand(const std::string& scriptInfo, const std::string& scriptFieldInfo, oo::ScriptValue pre_value, oo::ScriptValue post_value, oo::UUID gameobjID)
		:SI_name{ scriptInfo }, SFI_name{ scriptFieldInfo }, pre_val{ pre_value }, post_val{ post_value }, gameobjectID{ gameobjID }
	{
		message = SI_name + " Changed field " + SFI_name;
		PacketUtilts::BroadCastCommand(CommandPacketType::ActionScript, GetData());
	};
	Script_ActionCommand::~Script_ActionCommand()
	{
		SI_name.clear();
		SFI_name.clear();
	};
	void Script_ActionCommand::Undo()
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto go = scene->FindWithInstanceID(gameobjectID);
		auto& scriptinfoall = go->GetComponent<oo::ScriptComponent>().GetScriptInfoAll();
		auto scriptInfoIter = scriptinfoall.find(SI_name);
		if (scriptInfoIter == scriptinfoall.end())
		{
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "script not found");
			return;
		}

		auto scriptfield = scriptInfoIter->second.fieldMap.find(SFI_name);
		if (scriptfield == scriptInfoIter->second.fieldMap.end())
		{
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "script field not found");
			return;
		}
		scriptfield->second.value = pre_val;
	}
	void Script_ActionCommand::Redo()
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto go = scene->FindWithInstanceID(gameobjectID);
		auto& scriptinfoall = go->GetComponent<oo::ScriptComponent>().GetScriptInfoAll();
		auto scriptInfoIter = scriptinfoall.find(SI_name);
		if (scriptInfoIter == scriptinfoall.end())
		{
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "script not found");
			return;
		}

		auto scriptfield = scriptInfoIter->second.fieldMap.find(SFI_name);
		if (scriptfield == scriptInfoIter->second.fieldMap.end())
		{
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "script field not found");
			return;
		}
		scriptfield->second.value = post_val;
	}
	std::string oo::Script_ActionCommand::GetData()
	{
		std::string data;
		data += SI_name;
		data += PacketUtilts::SEPERATOR;
		data += SFI_name;
		data += PacketUtilts::SEPERATOR;
		data += std::to_string(gameobjectID.GetUUID());
		data += PacketUtilts::SEPERATOR;
		//getting the script values
		oo::ScriptFieldInfo info(SFI_name, pre_val);
		data += Serializer::SaveSingleScriptField(info);
		data += PacketUtilts::SEPERATOR;
		info.value = post_val;
		data += Serializer::SaveSingleScriptField(info);
		data += PacketUtilts::SEPERATOR;

		return data;
	}
	Script_ActionCommand::Script_ActionCommand(PacketHeader& packet, std::string& data)
	{
		size_t offset = 0;
		SI_name = PacketUtilts::ParseCommandData(data, offset);
		SFI_name = PacketUtilts::ParseCommandData(data, offset);
		gameobjectID = std::stoull(PacketUtilts::ParseCommandData(data, offset));
		//wip
		auto type = oo::ScriptClassInfo{ SI_name }.GetScriptFieldType(SFI_name);
		

		//get script field info
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto go = scene->FindWithInstanceID(gameobjectID);
		auto& scriptinfoall = go->GetComponent<oo::ScriptComponent>().GetScriptInfoAll();
		auto scriptInfoIter = scriptinfoall.find(SI_name);
		if (scriptInfoIter == scriptinfoall.end())
		{
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "script not found");
			return;
		}

		auto scriptfield = scriptInfoIter->second.fieldMap.find(SFI_name);
		if (scriptfield == scriptInfoIter->second.fieldMap.end())
		{
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "script field not found");
			return;
		}
		oo::ScriptFieldInfo info = scriptfield->second;

		std::string vals = PacketUtilts::ParseCommandData(data, offset);
		Serializer::LoadSingleScriptField(info, type, vals);
		pre_val = info.value;

		vals = PacketUtilts::ParseCommandData(data, offset);
		Serializer::LoadSingleScriptField(info, type, vals);
		post_val = info.value;

		message = "Script Fields Edited by : ";
		message += packet.name;
		Redo();
	}
}
