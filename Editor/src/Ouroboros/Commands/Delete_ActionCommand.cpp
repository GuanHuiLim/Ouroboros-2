#include "pch.h"
#include "Delete_ActionCommand.h"
#include "App/Editor/Serializer.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Scene/Scene.h"
#include "SceneManagement/include/SceneManager.h"
oo::Delete_ActionCommand::Delete_ActionCommand(std::shared_ptr<oo::GameObject> deletedObj)
	:data { Serializer::SaveDeletedObject(deletedObj, *ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()) }
{
	message = "Delete Object :" + deletedObj->Name();
	parentID = deletedObj->GetParentUUID();
}

oo::Delete_ActionCommand::~Delete_ActionCommand()
{
	message.clear();
	data.clear();
}

void oo::Delete_ActionCommand::Undo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	if (scene == nullptr)
	{
		ASSERT_MSG(true, "scene not found???");
	}

	revivedObject = Serializer::LoadDeleteObject(data, parentID, *(scene.get()));
}

void oo::Delete_ActionCommand::Redo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto obj = scene->FindWithInstanceID(revivedObject);
	if (obj == nullptr)
		ASSERT_MSG(true, "object not found");
	scene->DestroyGameObject(*obj);
}
