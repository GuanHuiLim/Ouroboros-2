#pragma once
#include "ActionCommand.h"
#include "Ouroboros/ECS/GameObject.h"
namespace oo
{
	class Ordering_ActionCommand : public ActionCommand
	{
	public:
		Ordering_ActionCommand(std::shared_ptr<oo::GameObject> go,oo::UUID new_parent);
		~Ordering_ActionCommand();
		void Undo()override;
		void Redo()override;
	private:
		oo::UUID old_parent;
		oo::UUID new_parent;
		oo::UUID targetObject;
	};
};
