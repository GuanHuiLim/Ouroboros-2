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

//#include "vk_engine.h"
struct SDL_Window;

namespace oo
{
    class VulkanContext : public GraphicsContext
    {
    public:
        VulkanContext(SDL_Window* window);
        virtual ~VulkanContext();

        void Init() override;
        void OnUpdateBegin() override;
        void SwapBuffers() override;

        void InitImGui() override;
        void OnImGuiBegin() override;
        void OnImGuiEnd() override;
        void OnImGuiShutdown() override;

        void SetWindowResized() override;
        bool SetVSync(bool enable) override;

        VulkanRenderer* getRenderer();

    private:
        SDL_Window* m_windowHandle;
        static VulkanRenderer* vr;
        static GraphicsWorld gw;
        static Window m_window;

        //TEMP ptrs
        std::unique_ptr<Model> cubeMesh;
        std::unique_ptr<Model> planeMesh;

        //static VulkanEngine vkEngine;
    };
}