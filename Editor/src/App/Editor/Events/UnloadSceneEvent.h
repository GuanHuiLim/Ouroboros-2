/************************************************************************************//*!
\file          UnloadSceneEvent.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Event Triggered when unloading a scene

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include "Ouroboros/Scene/Scene.h"
#include <filesystem>

class UnloadSceneEvent :public oo::Event
{
public:
	UnloadSceneEvent(oo::Scene* scene) :
		m_scene{ scene }
	{
	};

	~UnloadSceneEvent() {};

	oo::Scene* m_scene;
};

