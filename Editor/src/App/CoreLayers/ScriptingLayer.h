/************************************************************************************//*!
\file           ScriptingLayer.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Aug 22, 2022
\brief          Defines a layer to handle the global scripting functionality

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include "Ouroboros/Core/Layer.h"
#include <Scripting/Scripting.h>
#include "Ouroboros/Scripting/ScriptManager.h"

#include "Ouroboros/Transform/TransformComponent.h"

// FOR TESTING
#include "Ouroboros/Core/Input.h"
#include <filesystem>

namespace oo
{
    class ScriptingLayer final : public oo::Layer
    {
    public:

        ScriptingLayer(SceneManager const& sceneManager)
        {
            ScriptManager::RegisterComponent<TransformComponent>("Ouroboros", "Transform");

            ScriptManager::s_SceneManager = &sceneManager;
        }

        ~ScriptingLayer()
        {
            ScriptEngine::Shutdown();
        }

        void OnAttach() override final
        {

        }

        void OnDetach() override final
        {

        }

        void OnUpdate() override final
        {
        }
    };

}
