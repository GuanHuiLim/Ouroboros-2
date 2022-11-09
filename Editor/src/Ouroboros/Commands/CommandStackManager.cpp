#include "pch.h"
#include "CommandStackManager.h"

#include "App/Editor/UI/Tools/WarningMessage.h"
#include "App/Editor/Events/LoadSceneEvent.h"

#include <Ouroboros/Core/Assert.h>
#include "App/Editor/Networking/PacketUtils.h"

#include "Ouroboros/Commands/ActionCommandHelper.h"
#include "Ouroboros/Commands/Delete_ActionCommand.h"
#include "Ouroboros/Commands/Component_ActionCommand.h"
#include "Ouroboros/Commands/Ordering_ActionCommand.h"
#include "Ouroboros/Commands/ScriptAR_ActionCommand.h"
#include "Ouroboros/Commands/Script_ActionCommand.h"
#include "App/Editor/Networking/NetworkingEvent.h"
#include "Ouroboros/EventSystem/EventManager.h"
void oo::CommandStackManager::InitEvents()
{
	ActionCommandHelper::Init();
	EventManager::Subscribe<LoadSceneEvent>(
		[](LoadSceneEvent* lse)
		{
			//events that trigger a scene change
			CommandStackManager::ClearCommandBuffer();
		}
	);
	EventManager::Subscribe<NetworkingReceivedEvent>([](NetworkingReceivedEvent* e) {

		switch (CommandPacketType(e->header.packetType))
		{
		case CommandPacketType::ActionObject:
		{
			auto* ac = ActionCommandHelper::CreateActionCommand(e->header, e->data);
			CommandStackManager::AddCommand(e->header, ac);
		}break;
		case CommandPacketType::DeleteObject:
		{
			auto* dc = new Delete_ActionCommand(e->header, e->data);
			CommandStackManager::AddCommand(e->header, dc);
		}break;
		case CommandPacketType::CreateObject:
		{
			auto* cc = new Create_ActionCommand(e->header, e->data);
			CommandStackManager::AddCommand(e->header, cc);
		}break;
		case CommandPacketType::ParentObject: {
			auto* pc = new Parenting_ActionCommand(e->header, e->data);
			CommandStackManager::AddCommand(e->header, pc);
		}break;
		case CommandPacketType::ReorderObject: {
			auto* rc = new Ordering_ActionCommand(e->header, e->data);
			CommandStackManager::AddCommand(e->header, rc);
		}break;
		case CommandPacketType::AddComponentObject: {
			auto* ac = ActionCommandHelper::CreateActionCommand_Add(e->header, e->data);
			CommandStackManager::AddCommand(e->header, ac);
		}break;
		case CommandPacketType::RemoveComponentObject: {
			auto* ac = ActionCommandHelper::CreateActionCommand_Remove(e->header, e->data);
			CommandStackManager::AddCommand(e->header, ac);
		}break;
		case CommandPacketType::ActionScript: {
			auto* sac = new Script_ActionCommand(e->header, e->data);
			CommandStackManager::AddCommand(e->header, sac);
		}break;

		case CommandPacketType::AddScript: {
			auto* sa = new ScriptAdd_ActionCommand(e->header, e->data);
			CommandStackManager::AddCommand(e->header, sa);
		}break;
		case CommandPacketType::RemoveScript: {
			auto* sr = new ScriptRemove_ActionCommand(e->header, e->data);
			CommandStackManager::AddCommand(e->header, sr);
		}break;
		case CommandPacketType::UNDO_command:
		{
			UndoCommand(e->header);
		}break;
		case CommandPacketType::REDO_command:
		{
			RedoCommand(e->header);
		}break;
		case CommandPacketType::Selected_Object:
		{
			NetworkingSelectionEvent nse = NetworkingSelectionEvent(e->header, e->data);
			oo::EventManager::Broadcast(&nse);
		}break;
		};
		});
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

	PacketUtilts::BroadCastCommand(CommandPacketType::UNDO_command, "");
}

void oo::CommandStackManager::UndoCommand(PacketHeader& header)
{
	auto iter = s_networkMembers.find(header.name);
	if (iter == s_networkMembers.end())
	{
		ASSERT_MSG(true, "this shouldn't crash here");
		return;
	}
	ActionDeque& ad = iter->second;
	size_t idx = ad.commands.size() + ad.current;
	if (ad.commands.empty() || (idx) == 0)
		return;

	--ad.current;
	idx = ad.commands.size() + ad.current;

	ASSERT_MSG((idx >= ad.commands.size()), "Idx too big(overflow)");
	ad.commands[idx]->Undo();
	//display some undo message
	WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, ad.commands[idx]->ToString() + "(Undo)");
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
	PacketUtilts::BroadCastCommand(CommandPacketType::REDO_command, "");
}

void oo::CommandStackManager::RedoCommand(PacketHeader& header)
{
	auto iter = s_networkMembers.find(header.name);
	if (iter == s_networkMembers.end())
	{
		ASSERT_MSG(true, "this shouldn't crash here");
		return;
	}
	ActionDeque& ad = iter->second;
	if (ad.current == 0 || ad.commands.empty())
		return;
	size_t idx = ad.commands.size() + ad.current;
	if (idx + 1 > ad.commands.size())
		return;
	ASSERT_MSG((idx > ad.commands.size()), "Idx too big(overflow)");
	ad.commands[idx]->Redo();
	++ad.current;
	//display some redo message
	WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, ad.commands[idx]->ToString() + "(Redo)");

}

void oo::CommandStackManager::AddCommand(PacketHeader& header, ActionCommand* cmd)
{
	auto iter = s_networkMembers.find(header.name);
	if (iter == s_networkMembers.end())
	{
		s_networkMembers.emplace(header.name,ActionDeque());
		iter = s_networkMembers.find(header.name);//inefficient code
	}
	ActionDeque& ad = iter->second;
	while (ad.current < 0)
	{
		++ad.current;
		auto last_command = ad.commands.back();
		delete last_command;
		ad.commands.pop_back();
	}
	ad.current = 0;
	ad.commands.push_back(cmd);
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
	//clear network members
	for (auto& member : s_networkMembers)
	{
		member.second.current = 0;
		auto& commands = member.second.commands;
		while (commands.empty() == false)
		{
			oo::ActionCommand* command = commands.back();
			delete command;
			commands.pop_back();
		}
		commands.clear();
	}
	s_networkMembers.clear();
}
