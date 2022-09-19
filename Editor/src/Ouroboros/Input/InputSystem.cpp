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