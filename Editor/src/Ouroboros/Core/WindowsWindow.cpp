/************************************************************************************//*!
\file           WindowsWindow.cpp
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
#include "pch.h"

//#include <glad/glad.h>
#pragma warning (push)
#pragma warning (disable:26812)
#include <sdl2/SDL.h>
#pragma warning (pop)

#include "Ouroboros/Core/WindowsWindow.h"
//#include "Ouroboros/Core/Base.h"
#include "Ouroboros/Core/Application.h"

#include "Ouroboros/Vulkan/VulkanContext.h"

#include "Ouroboros/Core/Input.h"

#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

//#include <imgui_impl_sdl.h>
//#include <imgui.h>
#include <imgui/backends/imgui_impl_sdl.h>

#include "Events/ApplicationEvent.h"
#include "Events/ControllerEvent.h"
#include "Events/FileDropEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Ouroboros/EventSystem/EventManager.h"

// for accessibility access
#include "Accessibility.h"

namespace oo
{
    static bool s_SDLInitialized = false;

    WindowsWindow::WindowsWindow(const WindowProperties& props)
        : m_focused{ true }
        , m_minimized { false }
    {
        TRACY_PROFILE_SCOPE(windows_constructor);
        OPTICK_EVENT();
        Init(props);

        TRACY_PROFILE_SCOPE_END();
    }

    WindowsWindow::~WindowsWindow()
    {
        TRACY_PROFILE_SCOPE(windows_shutdown);
        OPTICK_EVENT();
        Shutdown();
        
        TRACY_PROFILE_SCOPE_END();
    }

    void WindowsWindow::Init(const WindowProperties& properties)
    {
        m_data = properties;
        /*m_data.Title = properties.Title;
        m_data.Width = properties.Width;
        m_data.Height = properties.Height;
        m_data.VSync = properties.VSync;
        m_data.FullScreen = properties.Fullscreen;*/

        LOG_CORE_INFO("Creating Windows window using SDL: [{0} {1}x{2}]", properties.Title, properties.Width, properties.Height);

        // windows creation
        if (!s_SDLInitialized)
        {
            TRACY_PROFILE_SCOPE(SDL_INIT);
            OPTICK_CATEGORY(SDL_INIT, Optick::Category::IO);
            int success = SDL_Init(SDL_INIT_VIDEO);
            UNREFERENCED_PARAMETER(success);
            //ASSERT_CUSTOM_MSG((success != 0), "Failed to initialize SDL {0}", SDL_GetError());
            ASSERT_MSG((success != 0), std::string{ "Failed to initialize SDL " } + SDL_GetError());
            s_SDLInitialized = true;
            TRACY_PROFILE_SCOPE_END();
        }

        // controller initialization
        {
            TRACY_PROFILE_SCOPE(CONTROLLER_INIT);
            OPTICK_CATEGORY(CONTROLLER_INIT, Optick::Category::IO);

            int success = SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER);
            UNREFERENCED_PARAMETER(success);
            ASSERT_MSG((success != 0), "Failed to initialize SDL {0}", SDL_GetError());
            //Load the gamecontrollerdb.txt and check if there was any problem
            int iNumOfControllers = SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
            //ASSERT_CUSTOM_MSG(iNumOfControllers == -1, "Error loading database {0}", SDL_GetError());
            ASSERT_MSG((iNumOfControllers == -1), std::string{ "Error loading database " } + SDL_GetError());
            UNREFERENCED(iNumOfControllers);

            // Ignore the controller events
            SDL_GameControllerEventState(SDL_IGNORE);

            TRACY_PROFILE_SCOPE_END();
        }


        SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
        window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN | window_flags);

        if (m_data.Fullscreen)
            window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_FULLSCREEN | window_flags);
        
        // Create windows
        {
            TRACY_PROFILE_SCOPE(SDL_CreateWindows);
            OPTICK_CATEGORY(SDL_CreateWindows, Optick::Category::IO);

            m_window = SDL_CreateWindow(m_data.Title.c_str()
                , SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED
                , m_data.Width, m_data.Height
                , window_flags);

            ASSERT_MSG(m_window == nullptr, "Failed to create SDL Window: {0}", SDL_GetError());

            TRACY_PROFILE_SCOPE_END();
        }

        {
    #ifdef OO_EDITOR
            // enable drag and drop functionality using SDL
            SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    #endif // EDITOR
        }

        {
            TRACY_PROFILE_SCOPE(Create_Vulkan_Context);
            OPTICK_CATEGORY(Create_Vulkan_Context, Optick::Category::Rendering);

            // create graphics context
            m_context = std::make_unique<VulkanContext>(m_window);
            m_context->Init();

            TRACY_PROFILE_SCOPE_END();
        }

        SDL_SetCursor(SDL_GetDefaultCursor());
        ShowCursor();

        // Set VSync Status
        SetVSync(m_data.VSync);
    }

    void WindowsWindow::Shutdown()
    {
        TRACY_PROFILE_SCOPE(Windows_Shutdown);
        OPTICK_EVENT();
        /* delete the current graphics context */
        //delete m_context;
        m_context.reset();

        SDL_DestroyWindow(m_window);
        SDL_Quit();

        TRACY_PROFILE_SCOPE_END();
        
        LOG_CORE_INFO("Finished Windows Shutdown");
    }

    void WindowsWindow::ProcessEvents()
    {
        TRACY_PROFILE_SCOPE(Process_Window_Events);
        OPTICK_EVENT();
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
#ifndef OO_END_PRODUCT
            // this should only run if there's imgui on
            ImGui_ImplSDL2_ProcessEvent(&event);
#endif 
            switch (event.type)
            {
                // WINDOWS EVENT
            case SDL_WINDOWEVENT:
            {
                switch (event.window.event)
                {
                    // Windows resize event
                    // both events are resize events
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                case SDL_WINDOWEVENT_RESIZED:
                {
                    if (event.window.windowID == SDL_GetWindowID(m_window)) // only care main window
                    {
                        m_data.Width = event.window.data1;
                        m_data.Height = event.window.data2;

                        WindowResizeEvent resizeEvent(m_data.Width, m_data.Height);
                        EventManager::Broadcast(&resizeEvent);
                    }
                    break;
                }
                //Windows close event
                case SDL_WINDOWEVENT_CLOSE:
                {
                    WindowCloseEvent closeEvent;
                    EventManager::Broadcast(&closeEvent);
                    break;
                }
                case SDL_WINDOWEVENT_MAXIMIZED:
                {
                    WindowMaximizeEvent windowMaximizeEvent;
                    EventManager::Broadcast(&windowMaximizeEvent);
                    
                    break;
                }
                case SDL_WINDOWEVENT_MINIMIZED:
                {
                    WindowMinimizeEvent windowMinimizeEvent;
                    EventManager::Broadcast(&windowMinimizeEvent);
                    m_minimized = true;
                    break;
                }
                case SDL_WINDOWEVENT_RESTORED:
                {
                    WindowRestoredEvent windowRestoredEvent;
                    EventManager::Broadcast(&windowRestoredEvent);
                    m_minimized = false;
                    break;
                }
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {
                    WindowFocusEvent windowFocusEvent;
                    EventManager::Broadcast(&windowFocusEvent);

                    m_focused = true;

                    break;
                }
                case SDL_WINDOWEVENT_FOCUS_LOST:
                {
                    WindowLoseFocusEvent windowLoseFocusEvent;
                    EventManager::Broadcast(&windowLoseFocusEvent);

                    m_focused = false;

                    break;
                }
                case SDL_WINDOWEVENT_MOVED:
                {
                    WindowMovedEvent windowMovedEvent;
                    EventManager::Broadcast(&windowMovedEvent);
                    break;
                }
                default:
                    break;
                }

                break;
            }

            case SDL_KEYDOWN:
            {
                KeyPressedEvent keyPressEvent((input::KeyCode)event.key.keysym.scancode, event.key.repeat ? 1 : 0);
                EventManager::Broadcast(&keyPressEvent);

                break;
            }
            case SDL_KEYUP:
            {
                KeyReleasedEvent keyPressEvent((input::KeyCode)event.key.keysym.scancode);
                EventManager::Broadcast(&keyPressEvent);

                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                MouseButtonReleasedEvent mouseButtonReleasedEvent((input::MouseCode)event.key.keysym.scancode);
                EventManager::Broadcast(&mouseButtonReleasedEvent);

                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                MouseButtonPressedEvent mouseButtonPressedEvent((input::MouseCode)event.key.keysym.scancode);
                EventManager::Broadcast(&mouseButtonPressedEvent);

                break;
            }
            case SDL_MOUSEWHEEL:
            {
                MouseScrolledEvent mouseScrolledEvent(static_cast<float>(event.wheel.x), static_cast<float>(event.wheel.y));
                EventManager::Broadcast(&mouseScrolledEvent);

                break;
            }
            case SDL_MOUSEMOTION:
            {
                MouseMovedEvent mouseMovedEvent(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));
                EventManager::Broadcast(&mouseMovedEvent);

                break;
            }
            //case SDL_JOYHATMOTION:  //back-bumpers
            case SDL_JOYBALLMOTION: //back-triggers
            case SDL_JOYAXISMOTION: //joystick
            {
                //Deadzone. can be improved.
                if (event.jaxis.value > -8000 && event.jaxis.value < 8000) break;

                std::cout << "ControllerMovedEvent on axis " << event.jaxis.axis << " and value of  " << event.jaxis.value << std::endl;

                ControllerMovedEvent controllerMovedEvent(event.jaxis.axis, event.jaxis.value);
                EventManager::Broadcast(&controllerMovedEvent);

                break;
            }
            case SDL_JOYBUTTONDOWN:
            {
                ControllerPressedEvent controllerPressedEvent(event.jbutton.button);
                EventManager::Broadcast(&controllerPressedEvent);

                break;
            }
            case SDL_JOYBUTTONUP:
            {
                ControllerReleasedEvent controllerReleasedEvent(event.jbutton.button);
                EventManager::Broadcast(&controllerReleasedEvent);

                break;
            }
            case SDL_JOYDEVICEADDED:
            {
                input::AddController(event.cdevice.which);
                ControllerAddedEvent controllerAddedEvent(event.cdevice.which);
                EventManager::Broadcast(&controllerAddedEvent);

                break;
            }
            case SDL_JOYDEVICEREMOVED:
            {
                input::RemoveController(event.cdevice.which);
                ControllerRemovedEvent controllerRemovedEvent(event.cdevice.which);
                EventManager::Broadcast(&controllerRemovedEvent);
                break;
            }
            //enable drag and drop in editor functionality
#ifdef OO_EDITOR
            case SDL_DROPBEGIN:
            {
                // event begin that triggers once on drop.
                // triggers only once for multiple file drops.
                FileDropEvent fileDrop(FileDropType::DropBegin, std::filesystem::path(), event.drop.windowID, event.drop.timestamp);
                //std::string fileDir(event.drop.file); <- drop file is null on begin/end
                LOG_CORE_INFO("file drop begin {0}", (uint32_t)event.drop.timestamp);
                break;
            }
            case SDL_DROPFILE:
            {
                // SDL_DropFile actually allocates memory for the char* on this event.
                // Triggers multiple times for multiple files.

                FileDropEvent fileDrop(FileDropType::DropFile, event.drop.file, event.drop.windowID, event.drop.timestamp);
                EventManager::Broadcast(&fileDrop);
                // get our dir as an RAII string
                std::string fileDir(event.drop.file);
                SDL_free(event.drop.file);    // Free dropped_filedir memory

                LOG_CORE_INFO("DroppedFile {0}, {1}", fileDir, (uint32_t)event.drop.timestamp);
                // Shows directory of dropped file

             //SDL_ShowSimpleMessageBox(
             //    SDL_MESSAGEBOX_INFORMATION,
             //    "File dropped on window",
             //    fileDir.c_str(),
             //    this->m_window
             //);
                break;
            }
#endif // OO_EDITOR

            default:
                break;
            }
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void WindowsWindow::ProcessWindowEvents()
    {
        TRACY_PROFILE_SCOPE(Process_Window_Events);
        OPTICK_EVENT();
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                // WINDOWS EVENT
            case SDL_WINDOWEVENT:
            {
                switch (event.window.event)
                {
                    // Windows resize event
                    // both events are resize events
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                case SDL_WINDOWEVENT_RESIZED:
                {
                    if (event.window.windowID == SDL_GetWindowID(m_window)) // only care main window
                    {
                        m_data.Width = event.window.data1;
                        m_data.Height = event.window.data2;

                        WindowResizeEvent resizeEvent(m_data.Width, m_data.Height);
                        EventManager::Broadcast(&resizeEvent);
                    }
                    break;
                }
                //Windows close event
                case SDL_WINDOWEVENT_CLOSE:
                {
                    WindowCloseEvent closeEvent;
                    EventManager::Broadcast(&closeEvent);
                    break;
                }
                case SDL_WINDOWEVENT_MAXIMIZED:
                {
                    WindowMaximizeEvent windowMaximizeEvent;
                    EventManager::Broadcast(&windowMaximizeEvent);

                    break;
                }
                case SDL_WINDOWEVENT_MINIMIZED:
                {
                    WindowMinimizeEvent windowMinimizeEvent;
                    EventManager::Broadcast(&windowMinimizeEvent);
                    m_minimized = true;
                    break;
                }
                case SDL_WINDOWEVENT_RESTORED:
                {
                    WindowRestoredEvent windowRestoredEvent;
                    EventManager::Broadcast(&windowRestoredEvent);
                    m_minimized = false;
                    break;
                }
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {
                    WindowFocusEvent windowFocusEvent;
                    EventManager::Broadcast(&windowFocusEvent);

                    m_focused = true;

                    break;
                }
                case SDL_WINDOWEVENT_FOCUS_LOST:
                {
                    WindowLoseFocusEvent windowLoseFocusEvent;
                    EventManager::Broadcast(&windowLoseFocusEvent);

                    m_focused = false;

                    break;
                }
                case SDL_WINDOWEVENT_MOVED:
                {
                    WindowMovedEvent windowMovedEvent;
                    EventManager::Broadcast(&windowMovedEvent);
                    break;
                }
                default:
                    break;
                }
            }
            break;

            default:
                break;
            }
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void WindowsWindow::SwapBuffers()
    {
        TRACY_PROFILE_SCOPE(Swapping_Buffers);
        OPTICK_EVENT();
        // swap rendering buffers
        m_context->SwapBuffers();

        TRACY_PROFILE_SCOPE_END();
    }

    void WindowsWindow::Maximize()
    {
        SDL_MaximizeWindow(m_window);
    }

    void WindowsWindow::Minimize()
    {
        SDL_MinimizeWindow(m_window);
    }

    void WindowsWindow::SetVSync(bool enabled)
    {
        LOG_CORE_INFO("Set Vsync : {0}", enabled);

        m_data.VSync = m_context->SetVSync(enabled);
    }

    void WindowsWindow::SetTitle(const std::string& title)
    {
        m_data.Title = title;
        SDL_SetWindowTitle(m_window, m_data.Title.c_str());
    }

    void WindowsWindow::SetFullScreen(bool fullscreen)
    {
        m_data.Fullscreen = fullscreen;
        accessibility::allow_accessibility_shortcut_keys(!fullscreen);
        SDL_SetWindowFullscreen(m_window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);  // 0 means non full-screen
    }

    void WindowsWindow::ShowCursor(bool showCursor)
    {
        showCursor ? SDL_ShowCursor(SDL_ENABLE) : SDL_ShowCursor(SDL_DISABLE);
    }

    void WindowsWindow::SetCursorGlobalPosition(int x, int y)
    {
        // returns 0 if supported.
        auto supported = SDL_WarpMouseGlobal(x, y);
        UNREFERENCED_PARAMETER(supported);
        ASSERT(supported);  // if not supported will break.
    }

    void WindowsWindow::SetCursorPosition(int x, int y)
    {
        SDL_WarpMouseInWindow(m_window, x, y);
    }

    void WindowsWindow::SetMouseLockState(bool lock)
    {
        auto result = SDL_SetRelativeMouseMode((SDL_bool)lock);
        ASSERT(result != 0);
        UNREFERENCED(result);
    }

    std::pair<int, int> WindowsWindow::GetWindowPos() const
    {
        int x, y;
        SDL_GetWindowPosition(m_window, &x, &y);
        return { x, y };
    }

    bool WindowsWindow::GetMouseCursorMode() const
    {
        return SDL_GetRelativeMouseMode();
    }

    bool WindowsWindow::IsVSync() const
    {
        return m_data.VSync;
    }

    bool WindowsWindow::IsFullscreen() const
    {
        return m_data.Fullscreen;
    }

    bool WindowsWindow::IsFocused() const
    {
        return m_focused;
    }

    bool WindowsWindow::IsMinimized() const
    {
        return m_minimized;
    }
}