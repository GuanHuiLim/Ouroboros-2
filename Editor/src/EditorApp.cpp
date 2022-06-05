/************************************************************************************//*!
\file           EditorApp.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Customer side of the project that utilizes the functions of the Engine.
                An Editor will be a use case for game engine.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"

// specially include this file only at the entry point of the engine.
#include <EntryPoint.h>

#include "Utility/Random.h"
#include <Ouroboros/Core/Input.h>
#include <Ouroboros/Core/Timer.h>
#include <Ouroboros/Core/LayerSet.h>

#include "TestLayers/InputDebugLayer.h"

class EditorApp final : public oo::Application
{
public:
    EditorApp(oo::CommandLineArgs args)
        : Application{ "Ouroboros v2.0", args }
    {
        m_layerset.PushLayer(std::make_shared<InputDebugLayer>());
    }

    void OnUpdate() override
    {
        m_layerset.Update();
        
        if (oo::input::IsKeyPressed(KEY_ESCAPE))
        {
            Close();
        }
    }

private:
    oo::LayerSet m_layerset;
};

oo::Application* oo::CreateApplication(oo::CommandLineArgs args)
{
    return new EditorApp{ args };
}
