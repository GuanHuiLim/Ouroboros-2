/************************************************************************************//*!
\file           InputManager.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 26, 2022
\brief          Declares and defines the manager responsible for handling all possible input axes
                available to the player

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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
        /*********************************************************************************//*!
        \brief      Used to initialize the list of input axes available to the player
                    to the default axes that are commonly used
        *//**********************************************************************************/
        static void LoadDefault();

        /*********************************************************************************//*!
        \brief      Used to initialize the list of input axes available to the player
                    to the list of input axes provided

        \param      loadedAxes
                the new list of input axes to use
        *//**********************************************************************************/
        static inline void Load(std::vector<InputAxis> loadedAxes)
        {
            axes = loadedAxes;
        }

        /*********************************************************************************//*!
        \brief      Initializes a container of trackers by creating and adding a tracker
                    for every input axis

        \param      trackers
                a reference to the trackers container to initialize
        *//**********************************************************************************/
        static inline void InitializeTrackers(std::unordered_map<std::string, InputAxis::Tracker>& trackers)
        {
            trackers.clear();
            for (InputAxis const& axis : axes)
            {
                trackers.emplace(axis.GetName(), InputAxis::Tracker{ axis });
            }
        }

        /*********************************************************************************//*!
        \brief      used to get a reference to the list of input axes available to the player

        \return     the list of input axes
        *//**********************************************************************************/
        static inline std::vector<InputAxis>& GetAxes()
        {
            return axes;
        }

    private:
        static inline std::vector<InputAxis> axes;
    };
}