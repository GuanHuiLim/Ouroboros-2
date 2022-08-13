#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include "Ouroboros/Scene/Scene.h"
#include <filesystem>
class LoadSceneEvent :public oo::Event
{
public:
	LoadSceneEvent( std::shared_ptr<oo::Scene> scene) :
		m_scene{scene}
	{
	};

	~LoadSceneEvent() {};

	std::shared_ptr<oo::Scene> m_scene;
};

