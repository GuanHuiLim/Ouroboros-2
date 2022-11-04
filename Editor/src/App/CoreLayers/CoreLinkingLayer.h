/************************************************************************************//*!
\file           CoreLinkingLayer.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 1, 2022
\brief          Defines a layer that will be running in the both the editor and
                the final distribution build that contains the main rendering scene

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include "Ouroboros/Core/Layer.h"
#include "App/Editor/Serializer.h"
#include "App/Editor/Properties/UI_RTTRType.h"

namespace oo
{
    // forward declare event
    struct GetCurrentSceneEvent;
    struct GetCurrentSceneStateEvent;

    class CoreLinkingLayer final : public oo::Layer
    {
    public:
        CoreLinkingLayer()
        {
            UI_RTTRType::Init();
            Serializer::Init();//runs the init function
            Serializer::InitEvents();
        }
        virtual ~CoreLinkingLayer() = default;
    };

}
