#include "pch.h"
#include "Delete_ActionCommand.h"
#include "App/Editor/Serializer.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Scene/Scene.h"
#include "SceneManagement/include/SceneManager.h"
#include "App/Editor/Networking/NetworkingEvent.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "App/Editor/Networking/PacketUtils.h"
oo::Delete_ActionCommand::Delete_ActionCommand(std::shared_ptr<oo::GameObject> deletedObj)
	:data { Serializer::SaveDeletedObject(deletedObj, *ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()) }
{
	message = "Delete Object :" + deletedObj->Name();
	parentID = deletedObj->GetParentUUID();
	revivedObject = deletedObj->GetInstanceID();
	PacketUtilts::BroadCastCommand(CommandPacketType::DeleteObject, GetData());
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
	data = Serializer::SaveDeletedObject(obj, *scene);
	scene->DestroyGameObject(*obj);
}

//networking stuff--------------------------------------------------//
std::string oo::Delete_ActionCommand::GetData()
{
	auto piD = parentID.GetUUID();
	std::string currData = std::to_string(piD);//its ok
	currData += PacketUtilts::SEPERATOR;
	piD = revivedObject.GetUUID();
	currData += std::to_string(piD);//its ok
 
	return currData;
}
oo::Delete_ActionCommand::Delete_ActionCommand(const PacketHeader& header, std::string& _data)
{
	size_t offset = 0;
	std::string temp_str = PacketUtilts::ParseCommandData(_data, offset);

	parentID = std::stoull(temp_str);
	revivedObject = std::stoull(std::string(_data.begin() + (offset), _data.end()));

	message += header.name;
	message += " Deleted Object : ";
	message += temp_str;

	Redo();
}

//create action
oo::Create_ActionCommand::Create_ActionCommand(std::shared_ptr<oo::GameObject> createdobj)
	:parentID(createdobj->GetParentUUID()),object(createdobj->GetInstanceID())
{
	message = "Created Object :" + createdobj->Name();
	PacketUtilts::BroadCastCommand(CommandPacketType::CreateObject, GetData());
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
//networking stuff--------------------------------------------------//
std::string oo::Create_ActionCommand::GetData()
{
	auto piD = parentID.GetUUID();
	std::string currData = std::to_string(piD);//its ok
	currData += PacketUtilts::SEPERATOR;
	piD = object.GetUUID();
	currData += std::to_string(piD);//its ok
	currData += PacketUtilts::SEPERATOR;

	if (data.empty())
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		currData += Serializer::SaveDeletedObject(scene->FindWithInstanceID(object), *scene);
	}
	else
		currData += data;

	return currData;
}
oo::Create_ActionCommand::Create_ActionCommand(const PacketHeader& header, const std::string& _data)
{
	size_t offset = 0;
	std::string temp_str = PacketUtilts::ParseCommandData(_data, offset);

	parentID = std::stoull(temp_str);
	object = std::stoull(PacketUtilts::ParseCommandData(_data, offset));
	data = std::string(_data.begin() + (offset), _data.end());

	message += header.name;
	message += " Created Object : ";
	message += temp_str;

	Redo();
}
