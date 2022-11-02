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
	revivedObject = deletedObj->GetInstanceID();
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
oo::Delete_ActionCommand::Delete_ActionCommand(std::string& _data)
{
	message = "Delete Object : ";
	size_t offset = _data.find(',');
	parentID = std::stoull(std::string(_data.begin(), _data.begin() + offset));
	data = std::string(_data.begin() + (offset+1), _data.end());
}
std::string oo::Delete_ActionCommand::GetData()
{
	auto piD = parentID.GetUUID();
	std::string currData = std::to_string(piD);//its ok
	currData += ',';
	currData += data;
	return currData;
}

oo::Create_ActionCommand::Create_ActionCommand(std::shared_ptr<oo::GameObject> createdobj)
	:parentID(createdobj->GetParentUUID()),object(createdobj->GetInstanceID())
{
	message = "Created Object :" + createdobj->Name();
}
oo::Create_ActionCommand::~Create_ActionCommand()
{
	message.clear();
	data.clear();
}

void oo::Create_ActionCommand::Undo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	data = Serializer::SaveDeletedObject(scene->FindWithInstanceID(object), *scene);

	auto obj = scene->FindWithInstanceID(object);
	if (obj == nullptr)
		ASSERT_MSG(true, "object not found");
	scene->DestroyGameObject(*obj);
}
void oo::Create_ActionCommand::Redo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	if (scene == nullptr)
	{
		ASSERT_MSG(true, "scene not found???");
	}

	Serializer::LoadDeleteObject(data, parentID, *(scene));
}
