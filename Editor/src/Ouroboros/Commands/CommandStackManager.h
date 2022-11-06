#pragma once
#include <deque>
#include <unordered_map>
#include "ActionCommand.h"
#include "App/Editor/Networking/PacketUtils.h"
namespace oo
{
class CommandStackManager
{
public:
	static void InitEvents();
	//ctrl z
	static void UndoCommand();
	static void UndoCommand(PacketHeader& header);
	//ctrl y
	static void RedoCommand();
	static void RedoCommand(PacketHeader& header);
	static void AddCommand(PacketHeader& header, ActionCommand* cmd);
	static void AddCommand(ActionCommand * command);
	static void ClearCommandBuffer();
	static void ClearCommandBuffer(PacketHeader& header);
private:
	inline static int s_current;
	inline static std::deque<oo::ActionCommand*> s_commands;
	struct ActionDeque
	{
		std::deque<oo::ActionCommand*> commands;
		int current = 0;
	};
	inline static std::unordered_map<std::string, ActionDeque > s_networkMembers;
};

}