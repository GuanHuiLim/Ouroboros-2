/************************************************************************************//*!
\file           AnimationComponent.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          This component allows a gameobject to have animations

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "AnimationComponent.h"

#include <rttr/registration>
namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<AnimationComponent>("AnimationComponent")
            .property("AnimationTreeName", &AnimationComponent::GetAnimationTreeName, &AnimationComponent::SetAnimationTree);
    }
}

