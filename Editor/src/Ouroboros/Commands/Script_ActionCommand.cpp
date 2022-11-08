#include "pch.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"

#include "Script_ActionCommand.h"
#include "App/Editor/UI/Tools/WarningMessage.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include <SceneManagement/include/SceneManager.h>

#include "Ouroboros/Scripting/ScriptComponent.h"
namespace oo
{
	Script_ActionCommand::Script_ActionCommand(const std::string& scriptInfo, const std::string& scriptFieldInfo, oo::ScriptValue pre_value, oo::ScriptValue post_value, oo::UUID gameobjID)
		:SI_name{ scriptInfo }, SFI_name{ scriptFieldInfo }, pre_val{ pre_value }, post_val{ post_value }, gameobjectID{ gameobjID }
	{
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

		return std::string();
	}
	Script_ActionCommand::Script_ActionCommand(PacketHeader& packet, std::string& data)
	{
	}
}
