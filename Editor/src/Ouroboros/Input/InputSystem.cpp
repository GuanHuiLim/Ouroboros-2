/************************************************************************************//*!
\file           InputSystem.cpp
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 26, 2022
\brief          Defines the input system that handles scene-specific trackers
                for all possible input axes available to the player

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include "InputSystem.h"

#include <Ouroboros/Core/Input.h>
#include <Ouroboros/Core/Timer.h>

namespace oo
{
    void InputSystem::Run(Ecs::ECSWorld* world)
    {
        for (auto& [axisName, tracker] : trackers)
        {
            tracker.Update(timer::dt());
        }
    }
    
    float InputSystem::GetAxis(std::string const& axisName)
    {
        auto search = trackers.find(axisName);
        if (search == trackers.end())
            throw std::exception{ (std::string{ "Input Axis not found: " } + axisName).c_str() };
        return search->second.GetValue();
    }
}