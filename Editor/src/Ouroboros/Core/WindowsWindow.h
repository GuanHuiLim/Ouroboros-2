/************************************************************************************//*!
\file           WindowsWindow.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 15, 2022
\brief          Describes a Windows(Platform) specific windows that implements
                the generic window interface.
                Currently using SDL as the backend abstraction.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <string>

//forward declaration
struct SDL_Window;

namespace oo
{
    /********************************************************************************//*!
     @brief     Properties that make up a window
    *//*********************************************************************************/
    struct WindowProperties
    {
        std::string Title = "Core Engine";
        unsigned int Width = 1600;
        unsigned int Height = 900;
        bool Fullscreen = false;
        bool VSync = true;
    };

    //forward declaration
    class VulkanContext;

    /********************************************************************************//*!
     @brief     Describes a Windows(Platform) specific windows that implements
                the generic window interface.
                Currently using SDL as the backend abstraction.

     @note      This class Should not be directly accessed but instead through the Window
                class and its interface functions in application.
    *//*********************************************************************************/
    class WindowsWindow final
    {
    public:
        /*-----------------------------------------------------------------------------*/
        /* Constructors and Destructors                                                */
        /*-----------------------------------------------------------------------------*/
        WindowsWindow(const WindowProperties& props);
        virtual ~WindowsWindow();

        /*-----------------------------------------------------------------------------*/
        /* Functions                                                                   */
        /*-----------------------------------------------------------------------------*/
        //double CalcDeltaTime();

        void ProcessEvents();
        void SwapBuffers();
        void Maximize();
        void Minimize();

        //void OnUpdate(Timestep dt) override;

        /*-----------------------------------------------------------------------------*/
        /* Getters                                                                     */
        /*-----------------------------------------------------------------------------*/
        unsigned int GetWidth() const { return m_data.Width; }
        unsigned int GetHeight() const { return m_data.Height; }
        std::pair<unsigned int, unsigned int> GetSize() const { return { m_data.Width, m_data.Height }; }
        std::pair<int, int> GetWindowPos() const;

        void* GetNativeWindow() const { return m_window; }
        //void* GetRenderingContext() const { return m_context; }
        VulkanContext* GetVulkanContext() const { return m_context.get(); }

        bool IsVSync() const;
        bool IsFullscreen() const;
        bool IsFocused() const;

        /*-----------------------------------------------------------------------------*/
        /* Setters                                                                     */
        /*-----------------------------------------------------------------------------*/
        void SetVSync(bool enabled);
        void SetTitle(const std::string& title);
        void SetFullScreen(bool fullscreen);
        void ShowCursor(bool show = true);

    private:
        void Init(const WindowProperties& properties);
        void Shutdown();

    private:
        SDL_Window* m_window;
        std::unique_ptr<VulkanContext> m_context;
        WindowProperties m_data;
        bool m_focused;
    };
}