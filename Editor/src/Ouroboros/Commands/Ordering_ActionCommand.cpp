#include "pch.h"
#include "Ordering_ActionCommand.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Scene/Scene.h"
#include "SceneManagement/include/SceneManager.h"
oo::Ordering_ActionCommand::Ordering_ActionCommand(std::shared_ptr<oo::GameObject> go, oo::UUID newparent)
	:old_parent{go->GetParentUUID()},new_parent{newparent},targetObject{go->GetInstanceID()}
{
	message = "Parented : " + go->Name();
}

oo::Ordering_ActionCommand::~Ordering_ActionCommand()
{
	message.clear();
}

void oo::Ordering_ActionCommand::Undo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto old = scene->FindWithInstanceID(old_parent);
	auto curr = scene->FindWithInstanceID(targetObject);
	old->AddChild(*curr, true);
}

void oo::Ordering_ActionCommand::Redo()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto new_p = scene->FindWithInstanceID(new_parent);
	auto curr = scene->FindWithInstanceID(targetObject);
	new_p->AddChild(*curr, true);
}
