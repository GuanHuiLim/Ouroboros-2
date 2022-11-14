#pragma once
#include "Ouroboros/Commands/ActionCommand.h"
#include <string>
#include <Utility/UUID.h>
#include "App/Editor/Networking/PacketUtils.h"
namespace oo
{
	class ScriptAdd_ActionCommand :public ActionCommand
	{
	public:
		ScriptAdd_ActionCommand(oo::UUID go,std::string _namespace, std::string _name);
		~ScriptAdd_ActionCommand();
		void Undo() override;
		void Redo() override;
		std::string GetData();
		ScriptAdd_ActionCommand(PacketHeader& header, std::string& data);
	private:
		std::string script_namespace;
		std::string script_name;
		oo::UUID instance;
	};
	class ScriptRemove_ActionCommand :public ActionCommand
	{
	public:
		ScriptRemove_ActionCommand(oo::UUID go, std::string _namespace, std::string _name);
		~ScriptRemove_ActionCommand();
		void Undo() override;
		void Redo() override;
		std::string GetData();
		ScriptRemove_ActionCommand(PacketHeader& header, std::string& data);
	private:
		std::string script_namespace;
		std::string script_name;
		oo::UUID instance;
	};
}
