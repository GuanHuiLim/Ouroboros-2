#pragma once

#include <vector>
#include <exception>

#include <Ouroboros/Core/KeyCode.h>
#include <Ouroboros/Core/MouseCode.h>

#include "InputAxis.h"

namespace oo
{
    class InputManager final
    {
    public:
        static void LoadDefault();

        static inline void Load(std::vector<InputAxis> loadedAxes)
        {
            axes = loadedAxes;
        }

        static inline void InitializeTrackers(std::unordered_map<std::string, InputAxis::Tracker>& trackers)
        {
            for (InputAxis const& axis : axes)
            {
                trackers.emplace(axis.GetName(), InputAxis::Tracker{ axis });
            }
        }

        static inline std::vector<InputAxis>& GetAxes()
        {
            return axes;
        }

    private:
        static inline std::vector<InputAxis> axes;
    };
}