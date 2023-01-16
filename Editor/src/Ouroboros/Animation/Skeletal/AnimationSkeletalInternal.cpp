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
    void TryTransitionIfPossible(SM_Instance& instance, AnimationSkeletonStateMachine const& statemachine)
    {
        if (instance.canTransition == false) return;

        instance.inTransition = true;
        instance.canTransition = false;
        instance.transitionTimer = 0.f;
		instance.transitionDuration = instance.transition->Duration();
        
        //consume triggers - as needed
		//ConsumeTriggers(instance, *instance.transition);

        instance.nextState = instance.transition->NextState();
        

    }
    void ConsumeTriggers(SM_Instance& instance, Transition const& transition)
    {
        for (auto const& rule : transition.Rules())
        {
            if (rule.IsTrigger() == false) continue;

			rule.GetParam(instance).get_value<Trigger>().Reset();
        }
    }
}


namespace oo::SkAnim
{

}