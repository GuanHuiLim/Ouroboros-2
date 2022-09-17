#pragma once

#include <unordered_map>
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

        static inline void Load(std::unordered_map<std::string, InputAxis> loadedAxes)
        {
            axes = loadedAxes;
        }

        static inline void InitializeTrackers(std::unordered_map<std::string, InputAxis::Tracker>& trackers)
        {
            for (auto const& [key, axis] : axes)
            {
                trackers.emplace(key, InputAxis::Tracker{ axis });
            }
        }

        static inline InputAxis const& GetAxis(std::string const& axisName)
        {
            auto search = axes.find(axisName);
            if (search == axes.end())
                throw std::exception{ (std::string{ "Input Axis not found: " } + axisName).c_str() };
            return search->second;
        }

    private:
        static inline std::unordered_map<std::string, InputAxis> axes;
    };
}