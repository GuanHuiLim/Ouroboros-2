#include "pch.h"
#include "Ordering_ActionCommand.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Scene/Scene.h"
#include "SceneManagement/include/SceneManager.h"

oo::Parenting_ActionCommand::Parenting_ActionCommand(std::shared_ptr<oo::GameObject> go, oo::UUID newparent)
	:old_parent{go->GetParentUUID()},new_parent{newparent},targetObject{go->GetInstanceID()}
{
	message = "Parented : " + go->Name();
	PacketUtilts::BroadCastCommand(CommandPacketType::ParentObject, GetData());
}

oo::Parenting_ActionCommand::~Parenting_ActionCommand()
{
	message.clear();
}

void oo::Parenting_ActionCommand::Undo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto old = scene->FindWithInstanceID(old_parent);
	auto curr = scene->FindWithInstanceID(targetObject);
	old->AddChild(*curr, true);
}

void oo::Parenting_ActionCommand::Redo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto new_p = scene->FindWithInstanceID(new_parent);
	auto curr = scene->FindWithInstanceID(targetObject);
	new_p->AddChild(*curr, true);
}

oo::Parenting_ActionCommand::Parenting_ActionCommand(PacketHeader& header, std::string& data)
{
	message = "Parented by : ";
	message += header.name;
	size_t offset = 0;
	old_parent = std::stoull(PacketUtilts::ParseCommandData(data, offset));
	new_parent = std::stoull(PacketUtilts::ParseCommandData(data, offset));
	targetObject = std::stoull(PacketUtilts::ParseCommandData(data, offset));

	Redo();
}

std::string oo::Parenting_ActionCommand::GetData()
{
	std::string data;
	data += std::to_string(old_parent);
	data += PacketUtilts::SEPERATOR;
	data += std::to_string(new_parent);
	data += PacketUtilts::SEPERATOR;
	data += std::to_string(targetObject);
	data += PacketUtilts::SEPERATOR;
	return data;
}

oo::Ordering_ActionCommand::Ordering_ActionCommand(std::shared_ptr<oo::GameObject> go, oo::UUID _target, bool move_to_after)
	:target{_target}, object{go->GetInstanceID()},redo_move_to_after{move_to_after}
{
	auto childs = go->GetParent().GetDirectChildsUUID();
	std::reverse(childs.begin(),childs.end());
	for (size_t i = 0; i < childs.size(); ++i)
	{
		if (childs[i] == object)
		{
			if (i == 0)//younger
			{
				undo_move_to_after = true;
				previous = childs[i+1];
			}
			else
			{
				undo_move_to_after = false;
				previous = childs[i-1];
			}
			break;
		}
	}
	message = "Reordering : " + go->Name();
	PacketUtilts::BroadCastCommand(CommandPacketType::ReorderObject, GetData());
}

oo::Ordering_ActionCommand::~Ordering_ActionCommand()
{
	message.clear();
}

void oo::Ordering_ActionCommand::Undo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto old = scene->FindWithInstanceID(previous);
	auto go = scene->FindWithInstanceID(object);
	go->GetSceneNode().lock()->move_to(old->GetSceneNode().lock(), undo_move_to_after);
}

void oo::Ordering_ActionCommand::Redo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto curr = scene->FindWithInstanceID(target);
	auto go = scene->FindWithInstanceID(object);
	go->GetSceneNode().lock()->move_to(curr->GetSceneNode().lock(), redo_move_to_after);
}

oo::Ordering_ActionCommand::Ordering_ActionCommand(PacketHeader& header, std::string& data)
{
	size_t offset = 0;
	object = std::stoull(PacketUtilts::ParseCommandData(data, offset));
	target = std::stoull(PacketUtilts::ParseCommandData(data, offset));
	previous = std::stoull(PacketUtilts::ParseCommandData(data, offset));
	redo_move_to_after = (bool)std::stoi(PacketUtilts::ParseCommandData(data, offset));
	undo_move_to_after = (bool)std::stoi(PacketUtilts::ParseCommandData(data, offset));
	message = "Reordered by :";
	message += header.name;
	Redo();
}

std::string oo::Ordering_ActionCommand::GetData()
{
	std::string data;
	data = std::to_string(object);
	data += PacketUtilts::SEPERATOR;
	data += std::to_string(target);
	data += PacketUtilts::SEPERATOR;
	data += std::to_string(previous);
	data += PacketUtilts::SEPERATOR;
	data += std::to_string(redo_move_to_after);
	data += PacketUtilts::SEPERATOR;
	data += std::to_string(undo_move_to_after);
	data += PacketUtilts::SEPERATOR;
	return data;
}
