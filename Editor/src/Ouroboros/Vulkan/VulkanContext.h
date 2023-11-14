/************************************************************************************//*!
\file           VulkanContext.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Jun 17, 2022
\brief          Describes the Vulkan Implementation of the Graphically Context
                and capabilities.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/core/GraphicsContext.h"

#include "OO_Vulkan/src/VulkanRenderer.h"
#include "OO_Vulkan/src/Window.h"

struct SDL_Window;

namespace oo
{
    // forward declaration
    class WindowResizeEvent;
    class WindowMinimizeEvent;
    class WindowRestoredEvent;

    class VulkanContext final //: public GraphicsContext
    {
    public:
        VulkanContext(SDL_Window* window);
        ~VulkanContext();

        void Init();
        void OnUpdateBegin();
        void SwapBuffers();

        void InitImGui();
        void ResetImguiInit();
        void OnImGuiBegin();
        void OnImGuiEnd();
        void ResetImguiShutdown();
        void OnImGuiShutdown();

        void SetWindowResized();
        bool SetVSync(bool enable);

        VulkanRenderer* getRenderer();

    public:
        SDL_Window* m_windowHandle;
        static VulkanRenderer* vr;
        static GraphicsWorld gw;
        static Window m_window;

        bool m_minimized = false;

        bool m_renderThreadRunning = true;
        std::thread m_renderThread;
    private:
        void OnWindowResize(WindowResizeEvent* e);
        void OnWindowMinimize(WindowMinimizeEvent* e);
        void OnWindowRestored(WindowRestoredEvent* e);
    };
}