/************************************************************************************//*!
\file           EventTest.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Sept 30, 2022
\brief          File to write sample functions and verify various event works as intended

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include <pch.h>

#include <Ouroboros/EventSystem/EventManager.h>
#include <Ouroboros/EventSystem/EventTypes.h>

// Include only those that you need
#include <Ouroboros/Scene/Scene.h>
#include <Ouroboros/Scene/RuntimeScene.h>
#include <Ouroboros/Scene/EditorScene.h>

#include <Ouroboros/Core/Base.h>
// Highly abusable way to get the scene
// Use with care.
void RetrieveScene()
{
    oo::GetCurrentSceneEvent e;
    oo::EventManager::Broadcast(&e);

    if (e.IsEditor)
    {
        auto& editor_scn = e.CurrentEditorScene;
        UNREFERENCED(editor_scn);
    }
    else
    {
        auto& runtime_scn = e.CurrentRuntimeScene; 
        UNREFERENCED(runtime_scn);
    }

    // Retrieve General Scene [if its good enough]
    auto& scn = e.CurrentScene;

    // Get ecs_world
    auto& ecs_world = scn->GetWorld();
    UNREFERENCED(ecs_world);
}