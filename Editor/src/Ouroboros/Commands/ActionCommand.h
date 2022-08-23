#pragma once
#include <string>
namespace oo
{
	class ActionCommand
	{
	public:
		ActionCommand() {};
		virtual ~ActionCommand() { message.clear(); };
		virtual void Undo() = 0;
		virtual void Redo() = 0;
		virtual std::string ToString() { return message; };
		std::string message = "";
	private:

	};
}