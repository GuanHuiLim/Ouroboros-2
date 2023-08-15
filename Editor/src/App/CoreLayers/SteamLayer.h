/************************************************************************************//*!
\file           EditorLayer.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 31, 2022
\brief          Defines a layer that will be running during editor mode
				and its related events

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Ouroboros/Core/Layer.h>
#include <Ouroboros/EventSystem/Event.h>

class SteamLayer final : public oo::Layer
{
private:

public:

public:
	SteamLayer()
	{


	}

	virtual void OnAttach() override final;

	virtual void OnUpdate() override final;

	virtual void OnDetach() override final;
};
