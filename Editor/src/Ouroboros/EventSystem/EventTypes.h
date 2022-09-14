/************************************************************************************//*!
\file           EventTypes.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552 | code contribution (100%)
\par            email: l.guanhui\@digipen.edu
\date           November 12, 2022
\brief          This is a header file intended for all desired event classes/structs derived
                from oo::event::Event that are shared between the engine and external
                users of the engine.
                They should not be defined here but rather in their own header files and
                ther header files should be included here.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/EventSystem/Event.h"

namespace oo
{
    //forward declare
    class EditorScene;
    class RuntimeScene;
    class Scene;

    struct GetCurrentSceneEvent : public Event
    {
        std::shared_ptr<EditorScene> CurrentEditorScene = nullptr;
        std::shared_ptr<RuntimeScene> CurrentRuntimeScene = nullptr;
        std::shared_ptr<Scene> CurrentScene = nullptr;
        bool IsEditor = true;
    };

}