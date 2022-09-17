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

        float GetAxis(std::string const& axisName);

    private:
        std::unordered_map<std::string, InputAxis::Tracker> trackers;
    };
}