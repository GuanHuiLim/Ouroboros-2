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

// TODO: remove
#include "Ouroboros/Core/CameraController.h"

//#include "vk_engine.h"
struct SDL_Window;


namespace oo
{
    // forward declaration
    class WindowResizeEvent;
    class WindowLoseFocusEvent;

    class VulkanContext //: public GraphicsContext
    {
    public:
        VulkanContext(SDL_Window* window);
        virtual ~VulkanContext();

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

    private:
        SDL_Window* m_windowHandle;
        static VulkanRenderer* vr;
        static GraphicsWorld gw;
        static Window m_window;
        CameraController m_cc;

        //TEMP ptrs
        std::unique_ptr<ModelData> cubeMesh;
        std::unique_ptr<ModelData> planeMesh;

    private:
        void OnWindowResize(WindowResizeEvent* e);
        void OnWindowLoseFocus(WindowLoseFocusEvent* e);
        //static VulkanEngine vkEngine;
    };
}