#include "pch.h"
#include "Ordering_ActionCommand.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Scene/Scene.h"
#include "SceneManagement/include/SceneManager.h"
oo::Parenting_ActionCommand::Parenting_ActionCommand(std::shared_ptr<oo::GameObject> go, oo::UUID newparent)
	:old_parent{go->GetParentUUID()},new_parent{newparent},targetObject{go->GetInstanceID()}
{
	message = "Parented : " + go->Name();
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

oo::Ordering_ActionCommand::Ordering_ActionCommand(std::shared_ptr<oo::GameObject> go, oo::UUID _target, bool move_to_after)
	:target{_target}, object{go->GetInstanceID()},redo_move_to_after{move_to_after}
{
	auto childs = go->GetParent().GetDirectChildsUUID();
	std::reverse(childs.begin(),childs.end());
	for (size_t i = 0; i < childs.size(); ++i)
	{
		if (childs[i] == object)
		{
			if (i > 0)
			{
				undo_move_to_after = false;
				previous = childs[i - 1];
			}
			else
			{
				undo_move_to_after = true;
				previous = childs[i];
			}
			break;
		}
	}
	message = "Parenting : " + go->Name();
}

oo::Ordering_ActionCommand::~Ordering_ActionCommand()
{
	message = "";
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
