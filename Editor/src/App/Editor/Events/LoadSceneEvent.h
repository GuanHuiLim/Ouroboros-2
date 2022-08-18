#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include "Ouroboros/Scene/Scene.h"
#include <filesystem>

class LoadSceneEvent :public oo::Event
{
public:
	LoadSceneEvent(oo::Scene* scene) :
		m_scene{scene}
	{
	};

	~LoadSceneEvent() {};

	oo::Scene* m_scene;
};

