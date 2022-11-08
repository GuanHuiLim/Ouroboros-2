#include "pch.h"
#include "ScriptAR_ActionCommand.h"
#include "Ouroboros/Scene/Scene.h"
#include "SceneManagement/include/SceneManager.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include <Ouroboros/Scripting/ScriptSystem.h>
oo::ScriptAdd_ActionCommand::ScriptAdd_ActionCommand(oo::UUID go, std::string _namespace, std::string _name)
	:instance{go},script_namespace{_namespace},script_name{_name}
{
	message = "Added Script :";
	message += script_namespace + script_name;
}

oo::ScriptAdd_ActionCommand::~ScriptAdd_ActionCommand()
{
	script_name.clear();
	script_namespace.clear();
	message.clear();
}

void oo::ScriptAdd_ActionCommand::Undo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto scriptsystem = scene->GetWorld().Get_System<oo::ScriptSystem>();
	auto gameobject = scene->FindWithInstanceID(instance);
	scriptsystem->RemoveScript(instance, script_namespace.c_str(), script_name.c_str());
	gameobject->GetComponent<oo::ScriptComponent>().RemoveScriptInfo(oo::ScriptClassInfo(script_namespace, script_name));

}

void oo::ScriptAdd_ActionCommand::Redo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto scriptsystem =	scene->GetWorld().Get_System<oo::ScriptSystem>();
	auto gameobject = scene->FindWithInstanceID(instance);
	scriptsystem->AddScript(instance, script_namespace.c_str(), script_name.c_str());
	gameobject->GetComponent<oo::ScriptComponent>().AddScriptInfo(oo::ScriptClassInfo(script_namespace,script_name));
}

std::string oo::ScriptAdd_ActionCommand::GetData()
{
	std::string data;
	data = script_namespace;
	data += PacketUtilts::SEPERATOR;
	data += script_name;
	data += PacketUtilts::SEPERATOR;
	data += std::to_string(instance);
	data += PacketUtilts::SEPERATOR;

    return data;
}

oo::ScriptAdd_ActionCommand::ScriptAdd_ActionCommand(PacketHeader& header, std::string& data)
{
	size_t offset = 0;
	script_namespace = PacketUtilts::ParseCommandData(data, offset);
	script_name = PacketUtilts::ParseCommandData(data, offset);
	instance = std::stoull(PacketUtilts::ParseCommandData(data, offset));

	message = header.name;
	message += " added ";
	message += script_namespace + script_name;
	Redo();
}

oo::ScriptRemove_ActionCommand::ScriptRemove_ActionCommand(oo::UUID go, std::string _namespace, std::string _name)
	:instance{ go }, script_namespace{ _namespace }, script_name{ _name }
{
	message = "Added Script :";
	message += script_namespace + script_name;
}

oo::ScriptRemove_ActionCommand::~ScriptRemove_ActionCommand()
{
	script_name.clear();
	script_namespace.clear();
	message.clear();
}

void oo::ScriptRemove_ActionCommand::Undo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto scriptsystem = scene->GetWorld().Get_System<oo::ScriptSystem>();
	auto gameobject = scene->FindWithInstanceID(instance);
	scriptsystem->AddScript(instance, script_namespace.c_str(), script_name.c_str());
	gameobject->GetComponent<oo::ScriptComponent>().AddScriptInfo(oo::ScriptClassInfo(script_namespace, script_name));
}

void oo::ScriptRemove_ActionCommand::Redo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto scriptsystem = scene->GetWorld().Get_System<oo::ScriptSystem>();
	auto gameobject = scene->FindWithInstanceID(instance);
	scriptsystem->RemoveScript(instance, script_namespace.c_str(), script_name.c_str());
	gameobject->GetComponent<oo::ScriptComponent>().RemoveScriptInfo(oo::ScriptClassInfo(script_namespace, script_name));
}

std::string oo::ScriptRemove_ActionCommand::GetData()
{
	std::string data;
	data = script_namespace;
	data += PacketUtilts::SEPERATOR;
	data += script_name;
	data += PacketUtilts::SEPERATOR;
	data += std::to_string(instance);
	data += PacketUtilts::SEPERATOR;

	return data;
}

oo::ScriptRemove_ActionCommand::ScriptRemove_ActionCommand(PacketHeader& header, std::string& data)
{
	size_t offset = 0;
	script_namespace = PacketUtilts::ParseCommandData(data, offset);
	script_name = PacketUtilts::ParseCommandData(data, offset);
	instance = std::stoull(PacketUtilts::ParseCommandData(data, offset));

	message = header.name;
	message += " added ";
	message += script_namespace + script_name;
	Redo();
}
