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

#include "Ouroboros/Core/Events/MouseEvent.h"
#include "Ouroboros/EventSystem/EventManager.h"

namespace oo
{
    class InputSystem final : public Ecs::System
    {
    public:
        InputSystem()
        {
            EventManager::Subscribe<InputSystem, MouseScrolledEvent>(this, &InputSystem::OnMouseScroll);
        }

        virtual ~InputSystem()
        {
            EventManager::Unsubscribe<InputSystem, MouseScrolledEvent>(this, &InputSystem::OnMouseScroll);
        }

        inline void Initialize()
        {
            InputManager::InitializeTrackers(trackers);
        }

        virtual void Run(Ecs::ECSWorld* world) override;

        inline void LateUpdate()
        {
            scrollValue = 0.0f;
        }

        InputAxis& GetAxis(std::string const& axisName);

        inline std::unordered_map<std::string, InputAxis::Tracker> const& GetTrackers()
        {
            return trackers;
        }

        float GetAxisValue(std::string const& axisName);

        static inline float GetScrollValue()
        {
            return scrollValue;
        }

    private:
        void OnMouseScroll(MouseScrolledEvent* e)
        {
            scrollValue = e->GetY();
        }

        std::unordered_map<std::string, InputAxis::Tracker> trackers;
        static inline float scrollValue;
    };
}