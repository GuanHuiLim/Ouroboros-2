#pragma once
#include <deque>
#include "ActionCommand.h"
namespace oo
{
class CommandStackManager
{
public:
	//ctrl z
	static void UndoCommand();
	//ctrl y
	static void RedoCommand();
	static void AddCommand(ActionCommand * command);
	static void ClearCommandBuffer();
private:
	inline static int s_current;
	inline static std::deque<oo::ActionCommand*> s_commands;
};

}