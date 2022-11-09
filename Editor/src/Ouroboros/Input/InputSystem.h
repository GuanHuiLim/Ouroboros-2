/************************************************************************************//*!
\file           InputSystem.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 26, 2022
\brief          Declares and defines the input system that handles scene-specific trackers 
                for all possible input axes available to the player

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include "InputManager.h"
#include "Ouroboros/ECS/GameObject.h"

namespace oo
{
    class InputSystem final : public Ecs::System
    {
    public:
        InputSystem() {}
        virtual ~InputSystem() = default;

        inline void Initialize()
        {
            InputManager::InitializeTrackers(trackers);
        }

        virtual void Run(Ecs::ECSWorld* world) override;

        InputAxis& GetAxis(std::string const& axisName);

        float GetAxisValue(std::string const& axisName);

    private:
        std::unordered_map<std::string, InputAxis::Tracker> trackers;
    };
}