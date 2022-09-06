/************************************************************************************//*!
\file           ImGuiAbstraction.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 25, 2022
\brief          Implements a basic ImGuiAbstraction that sets up for the core ImGui Features
                Required into the main application.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

//forward declaration
struct SDL_Window;

namespace oo
{
    //forward declaration
    //class GraphicsContext;
    class VulkanContext;

    class ImGuiAbstraction final
    {
    public:
        /*-----------------------------------------------------------------------------*/
        /* Constructors and Destructors                                                */
        /*-----------------------------------------------------------------------------*/
        ImGuiAbstraction();
        ~ImGuiAbstraction();

        /*-----------------------------------------------------------------------------*/
        /* Functions                                                                   */
        /*-----------------------------------------------------------------------------*/
        void Init();
        void Destroy();
        
        void Begin();
        void End();
        
        void Restart() { m_restart = true; };

    private:
        bool m_restart;
        SDL_Window* m_window;
        //GraphicsContext* m_renderer;
        VulkanContext* m_renderer;
    };

}