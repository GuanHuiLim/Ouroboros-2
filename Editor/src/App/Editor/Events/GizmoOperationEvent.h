#pragma once
#pragma once
#include "Ouroboros/EventSystem/Event.h"
struct GizmoOperationEvent : public oo::Event
{
	GizmoOperationEvent() {};
	int currOperation = 0;
};

struct ChangeGizmoEvent :public oo::Event
{
	ChangeGizmoEvent(int op) :targetOperation{ op } {};
	int targetOperation;
};
