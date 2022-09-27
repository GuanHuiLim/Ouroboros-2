#include "pch.h"
#include "CommandStackManager.h"

#include "App/Editor/UI/Tools/WarningMessage.h"
#include "App/Editor/Events/OpenFileEvent.h"

#include <Ouroboros/Core/Assert.h>
#include <Ouroboros/EventSystem/EventManager.h>
void oo::CommandStackManager::InitEvents()
{
	EventManager::Subscribe<OpenFileEvent>(
		[](OpenFileEvent* ofe)
		{
			//events that trigger a scene change
			if (ofe->m_type == OpenFileEvent::FileType::SCENE || ofe->m_type == OpenFileEvent::FileType::PREFAB)
				CommandStackManager::ClearCommandBuffer();
		}
	);
}
void oo::CommandStackManager::UndoCommand()
{
	if (s_commands.empty() || (s_current + s_commands.size()) == 0)
		return;
	--s_current;
	size_t idx = s_commands.size() + s_current;
	
	ASSERT_MSG((idx >= s_commands.size()), "Idx too big(overflow)");
	s_commands[idx]->Undo();
	//display some undo message
	WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, s_commands[idx]->ToString() + "(Undo)");
}

void oo::CommandStackManager::RedoCommand()
{
	if (s_current == 0 || s_commands.empty())
		return;
	size_t idx = s_commands.size() + s_current;
	if (idx + 1 > s_commands.size())
		return;
	ASSERT_MSG((idx > s_commands.size()), "Idx too big(overflow)");
	s_commands[idx]->Redo();
	++s_current;
	//display some redo message
	WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, s_commands[idx]->ToString() + "(Redo)");
}

void oo::CommandStackManager::AddCommand(ActionCommand* command)
{
	while (s_current < 0)
	{
		++s_current;
		auto last_command = s_commands.back();
		delete last_command;
		s_commands.pop_back();
	}
	s_current = 0;
	s_commands.push_back(command);
}

void oo::CommandStackManager::ClearCommandBuffer()
{
	s_current = 0;
	while (s_commands.empty() == false)
	{
		oo::ActionCommand* command = s_commands.back();
		delete command;
		s_commands.pop_back();
	}
	s_commands.clear();
}
