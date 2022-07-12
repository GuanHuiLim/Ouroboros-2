/************************************************************************************//*!
\file          DefaultTestScene.h
\project       Ouroboros
\author        Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par           email: c.tecklee\@digipen.edu
\date          July 27, 2022
\brief         Default empty Test scene

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/Scene/Scene.h"

class DefaultTestScene final : public oo::Scene
{
private:
    inline static bool m_print_debug_msgs = true;

public:

    #define PRINT(name) std::cout << "[" << (name) << "] : " << __FUNCTION__ << std::endl;

    DefaultTestScene(bool print_debug_msgs = true)
        : Scene{ "Default Test Scene" } 
    { 
        m_print_debug_msgs = print_debug_msgs;
    }
    
    virtual ~DefaultTestScene() = default;

    virtual void Init() override final 
    { 
        if(m_print_debug_msgs) 
            PRINT(GetSceneName()); 
    }

    virtual void Update() override final 
    { 
        if (m_print_debug_msgs) 
            PRINT(GetSceneName()); 
    }

    virtual void LateUpdate() override final 
    {
        if (m_print_debug_msgs) 
            PRINT(GetSceneName()); 
    }

    virtual void Render() override final 
    {
        if (m_print_debug_msgs) 
            PRINT(GetSceneName()); 
    }
    
    virtual void EndOfFrameUpdate() override final 
    {
        if(m_print_debug_msgs) 
            PRINT(GetSceneName()); 
    }
    
    virtual void Exit() override final 
    {
        if (m_print_debug_msgs) 
            PRINT(GetSceneName()); 
    }


    virtual void LoadScene() override final
    {
        if (m_print_debug_msgs)
            PRINT(GetSceneName());
    }

    virtual void UnloadScene() override final
    {
        if (m_print_debug_msgs)
            PRINT(GetSceneName());
    }
    
    virtual void ReloadScene() override final
    {
        m_print_debug_msgs = !m_print_debug_msgs;
        LOG_INFO("Debugging Messages : {0}", m_print_debug_msgs);

        if (m_print_debug_msgs)
            PRINT(GetSceneName());
    }
};
