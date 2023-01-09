/************************************************************************************//*!
\file           AnimationSkeletalInternal.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          Internal functions for skeletal animation.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "AnimationSkeletalInternal.h"
#include "AnimationSkeletonStateMachine.h"

namespace oo::SkAnim::internal
{
    void CheckTransitionConditions(SM_Instance& instance, AnimationSkeletonStateMachine const& statemachine)
    {
        auto currState = statemachine.GetState(instance.currentState);
        if (currState == nullptr) return;

        for (auto const& transition : currState->transitions)
        {
            if (transition.CheckConditions(instance))
            {
                instance.canTransition = true;
                instance.transition = &transition;
                break;
            }
        }
    
    }
}


namespace oo::SkAnim
{

}