#pragma once
#include "ActionCommand.h"
#include "Ouroboros/ECS/GameObject.h"

namespace oo
{
	class Delete_ActionCommand:public ActionCommand
	{
	public:
		Delete_ActionCommand(std::shared_ptr<oo::GameObject> deletedObj);
		~Delete_ActionCommand();

		void Undo()override;
		void Redo()override;
	private:
		oo::UUID parentID;
		oo::UUID revivedObject = oo::UUID(static_cast<uint64_t>(-1));
		std::string data;
	};

}
