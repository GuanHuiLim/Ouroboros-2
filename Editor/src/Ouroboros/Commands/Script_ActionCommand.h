#pragma once
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/Scripting/ScriptInfo.h"
#include "Ouroboros/Scripting/ScriptComponent.h"
#include "App/Editor/Utility/ImGuiManager.h"

#include <SceneManagement/include/SceneManager.h>

#include "ActionCommand.h"

#include "App/Editor/UI/Tools/WarningMessage.h"
#include <string>
namespace oo
{

class Script_ActionCommand :public ActionCommand
{
public:
	Script_ActionCommand(const std::string& scriptInfo, const std::string& scriptFieldInfo, oo::ScriptValue pre_value, oo::ScriptValue post_value, UUID gameobjID)
		:SI_name{ scriptInfo }, SFI_name{ scriptFieldInfo }, pre_val{ pre_value }, post_val{ post_value }, gameobjectID{gameobjID}
	{
	};
	~Script_ActionCommand()
	{
		SI_name.clear();
		SFI_name.clear();
	};
	void Undo() override
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto go = scene->FindWithInstanceID(gameobjectID);
		auto& scriptinfoall = go->GetComponent<oo::ScriptComponent>().GetScriptInfoAll();
		auto scriptInfoIter = scriptinfoall.find(SI_name);
		if (scriptInfoIter == scriptinfoall.end())
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "script not found");
		
		auto scriptfield = scriptInfoIter->second.fieldMap.find(SFI_name);
		if(scriptfield == scriptInfoIter->second.fieldMap.end())
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "script field not found");
		scriptfield->second.value = pre_val;
	}
	void Redo() override
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto go = scene->FindWithInstanceID(gameobjectID);
		auto& scriptinfoall = go->GetComponent<oo::ScriptComponent>().GetScriptInfoAll();
		auto scriptInfoIter = scriptinfoall.find(SI_name);
		if (scriptInfoIter == scriptinfoall.end())
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "script not found");

		auto scriptfield = scriptInfoIter->second.fieldMap.find(SFI_name);
		if (scriptfield == scriptInfoIter->second.fieldMap.end())
			WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR, "script field not found");
		scriptfield->second.value = post_val;
	}
private:
	std::string SI_name;
	std::string SFI_name;
	oo::ScriptValue pre_val;
	oo::ScriptValue post_val;
	UUID gameobjectID;
};

}