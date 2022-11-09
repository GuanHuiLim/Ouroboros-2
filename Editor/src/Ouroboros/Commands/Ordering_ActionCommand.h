#pragma once
#include <string>
#include "ActionCommand.h"
#include "Ouroboros/ECS/GameObject.h"
#include "App/Editor/Networking/PacketUtils.h"
namespace oo
{
	class Parenting_ActionCommand : public ActionCommand
	{
	public:
		Parenting_ActionCommand(std::shared_ptr<oo::GameObject> go,oo::UUID new_parent);
		~Parenting_ActionCommand();
		void Undo()override;
		void Redo()override;
		Parenting_ActionCommand(PacketHeader& header, std::string& data);
	private:
		std::string GetData();
		oo::UUID old_parent;
		oo::UUID new_parent;
		oo::UUID targetObject;
	};

	class Ordering_ActionCommand : public ActionCommand
	{
	public:
		Ordering_ActionCommand(std::shared_ptr<oo::GameObject> go, oo::UUID target ,bool move_to_after);
		~Ordering_ActionCommand();
		void Undo()override;
		void Redo()override;
		Ordering_ActionCommand(PacketHeader& header, std::string& data);
	private:
		std::string GetData();
		oo::UUID object;
		oo::UUID target;
		oo::UUID previous;
		bool redo_move_to_after = true;
		bool undo_move_to_after = true;
	};

};
