#include "pch.h"
#include "Delete_ActionCommand.h"
#include "App/Editor/Serializer.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Scene/Scene.h"
oo::Delete_ActionCommand::Delete_ActionCommand(std::shared_ptr<oo::GameObject> deletedObj)
	:data { Serializer::SaveDeletedObject(deletedObj, *ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()) }
{
	message = "Delete Object :" + deletedObj->Name();
}

oo::Delete_ActionCommand::~Delete_ActionCommand()
{
	message.clear();
	data.clear();
}

void oo::Delete_ActionCommand::Undo()
{
	revivedObject = Serializer::LoadDeleteObject(data, parentID, *ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>());
}

void oo::Delete_ActionCommand::Redo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto obj = scene->FindWithInstanceID(revivedObject);
	if (obj == nullptr)
		ASSERT_MSG(true, "object not found");
	scene->DestroyGameObject(*obj);
}
