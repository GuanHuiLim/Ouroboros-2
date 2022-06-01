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
//#pragma warning (push)
//#pragma warning (disable:26812)
//#include <sdl2/SDL.h>
//#pragma warning (pop)

#include "Ouroboros/Core/WindowsWindow.h"
//#include "Ouroboros/Core/Base.h"
#include "Ouroboros/Core/Application.h"

//#if defined(GRAPHICS_CONTEXT_VULKAN)
//#include "Ouroboros/Platform/Vulkan/VulkanContext.h"
//#elif defined(GRAPHICS_CONTEXT_OPENGL)
//#include "Ouroboros/Platform/OpenGL/OpenGLContext.h"
//#endif

#include "Ouroboros/Core/Input.h"

//#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

//#include <imgui_impl_sdl.h>
//#include <imgui.h>

namespace oo
{
    static bool s_SDLInitialized = false;

    WindowsWindow::WindowsWindow(const WindowProperties& props)
        : m_focused{ true }
    {
        //TRACY_PROFILE_SCOPE("windows constructor");

        Init(props);

        //TRACY_PROFILE_SCOPE_END();
    }

    WindowsWindow::~WindowsWindow()
    {
        //TRACY_PROFILE_SCOPE("windows shutdown");

        Shutdown();
        
        //TRACY_PROFILE_SCOPE_END();
    }

    void WindowsWindow::Init(const WindowProperties& properties)
    {
        m_data.Title = properties.Title;
        m_data.Width = properties.Width;
        m_data.Height = properties.Height;
        m_data.VSync = properties.VSync;
        m_data.FullScreen = properties.Fullscreen;

//        LOG_ENGINE_INFO("Creating Windows window using SDL: [{0} {1}x{2}]", properties.Title, properties.Width, properties.Height);
//
//        // windows creation
//        if (!s_SDLInitialized)
//        {
//            TRACY_PROFILE_SCOPE("SDL_INIT");
//
//            int success = SDL_Init(SDL_INIT_VIDEO);
//            ENGINE_ASSERT_CUSTOM_MSG((success == 0), "Failed to initialize SDL {0}", SDL_GetError());
//
//            s_SDLInitialized = true;
//            TRACY_PROFILE_SCOPE_END();
//        }
//
//        // controller initialization
//        {
//            TRACY_PROFILE_SCOPE("CONTROLLER_INIT");
//
//            int success = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
//            ENGINE_ASSERT_MSG((success == 0), "Failed to initialize SDL {0}", SDL_GetError());
//            //Load the gamecontrollerdb.txt and check if there was any problem
//            int iNumOfControllers = SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
//            ENGINE_ASSERT_CUSTOM_MSG(iNumOfControllers != -1, "Error loading database {0}", SDL_GetError());
//
//            //// Ignore the controller events
//            SDL_GameControllerEventState(SDL_IGNORE);
//
//            TRACY_PROFILE_SCOPE_END();
//        }
//
//        //ENGINE_PROFILE_SCOPE("SDL_CreateWindows");
//
//#ifdef GRAPHICS_CONTEXT_OPENGL
//        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
//        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
//        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//
//        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
//        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
//#endif
//        SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
//
//#ifdef GRAPHICS_CONTEXT_VULKAN
//        window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN | window_flags);
//#elif  defined(GRAPHICS_CONTEXT_OPENGL)
//        window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_OPENGL | window_flags);
//#endif
//        if (m_data.FullScreen)
//            window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_FULLSCREEN | window_flags);
//
//        m_window = SDL_CreateWindow(m_data.Title.c_str()
//            , SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED
//            , m_data.Width, m_data.Height
//            , window_flags);
//
//        ENGINE_ASSERT_MSG(m_window, "Failed to create SDL Window: {0}", SDL_GetError());
//
//#ifdef OO_EDITOR
//        // enable drag and drop functionality using SDL
//        SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
//#endif // EDITOR
//
//        {
//            TRACY_PROFILE_SCOPE("Create And Initialize Context");
//
//            // create graphics context
//#ifdef GRAPHICS_CONTEXT_VULKAN
//            m_context = new VulkanContext(m_window);
//#elif  defined(GRAPHICS_CONTEXT_OPENGL)
//            m_context = new OpenGLContext(m_window);
//#endif
//            m_context->Init();
//
//            TRACY_PROFILE_SCOPE_END();
//        }
//
//        SDL_SetCursor(SDL_GetDefaultCursor());
//        ShowCursor();
//
//        // Set VSync Status
//        SetVSync(m_data.VSync);
    }

    void WindowsWindow::Shutdown()
    {
        //TRACY_PROFILE_SCOPE("Windows Shutdown");

        ///* delete the current graphics context */
        //delete m_context;

        //SDL_DestroyWindow(m_window);
        //SDL_Quit();

        //TRACY_PROFILE_SCOPE_END();
    }

    void WindowsWindow::ProcessEvents()
    {
//        TRACY_PROFILE_SCOPE("Process Events");
//
//        SDL_Event event;
//        while (SDL_PollEvent(&event))
//        {
//#ifdef OO_EDITOR
//            // this should only run if there's imgui on
//            ImGui_ImplSDL2_ProcessEvent(&event);
//#endif 
//            switch (event.type)
//            {
//                // WINDOWS EVENT
//            case SDL_WINDOWEVENT:
//            {
//                switch (event.window.event)
//                {
//                    //Windows resize event
//                    // both events are resize events
//                case SDL_WINDOWEVENT_SIZE_CHANGED:
//                case SDL_WINDOWEVENT_RESIZED:
//                {
//                    if (event.window.windowID == SDL_GetWindowID(m_window)) // only care main window
//                    {
//                        m_data.Width = event.window.data1;
//                        m_data.Height = event.window.data2;
//
//                        WindowResizeEvent resizeEvent(m_data.Width, m_data.Height);
//                        m_data.EventCallback(resizeEvent);
//                    }
//                    break;
//                }
//                //Windows close event
//                case SDL_WINDOWEVENT_CLOSE:
//                {
//                    WindowCloseEvent closeEvent;
//                    m_data.EventCallback(closeEvent);
//                    break;
//                }
//                case SDL_WINDOWEVENT_MAXIMIZED:
//                case SDL_WINDOWEVENT_FOCUS_GAINED:
//                {
//                    WindowFocusEvent windowFocusEvent;
//                    m_data.EventCallback(windowFocusEvent);
//
//                    m_focused = true;
//
//                    break;
//                }
//                case SDL_WINDOWEVENT_MINIMIZED:
//                case SDL_WINDOWEVENT_FOCUS_LOST:
//                {
//                    WindowLoseFocusEvent windowLoseFocusEvent;
//                    m_data.EventCallback(windowLoseFocusEvent);
//
//                    m_focused = false;
//
//                    break;
//                }
//                case SDL_WINDOWEVENT_MOVED:
//                {
//                    WindowMovedEvent windowMovedEvent;
//                    m_data.EventCallback(windowMovedEvent);
//                    break;
//                }
//                default:
//                    break;
//                }
//
//                break;
//            }
//
//            case SDL_KEYDOWN:
//            {
//                KeyPressedEvent keyPressEvent((KeyCode)event.key.keysym.scancode, event.key.repeat ? 1 : 0);
//                m_data.EventCallback(keyPressEvent);
//
//                break;
//            }
//            case SDL_KEYUP:
//            {
//                KeyReleasedEvent keyPressEvent((KeyCode)event.key.keysym.scancode);
//                m_data.EventCallback(keyPressEvent);
//
//                break;
//            }
//            case SDL_MOUSEBUTTONUP:
//            {
//                MouseButtonReleasedEvent mouseButtonReleasedEvent((MouseCode)event.key.keysym.scancode);
//                m_data.EventCallback(mouseButtonReleasedEvent);
//
//                break;
//            }
//            case SDL_MOUSEBUTTONDOWN:
//            {
//                MouseButtonPressedEvent mouseButtonPressedEvent((MouseCode)event.key.keysym.scancode);
//                m_data.EventCallback(mouseButtonPressedEvent);
//
//                break;
//            }
//            case SDL_MOUSEWHEEL:
//            {
//                MouseScrolledEvent mouseScrolledEvent(static_cast<float>(event.wheel.x), static_cast<float>(event.wheel.y));
//                m_data.EventCallback(mouseScrolledEvent);
//
//                break;
//            }
//            case SDL_MOUSEMOTION:
//            {
//                MouseMovedEvent mouseMovedEvent(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));
//                m_data.EventCallback(mouseMovedEvent);
//
//                break;
//            }
//            //case SDL_JOYHATMOTION:  //back-bumpers
//            case SDL_JOYBALLMOTION: //back-triggers
//            case SDL_JOYAXISMOTION: //joystick
//            {
//                //Deadzone. can be improved.
//                if (event.jaxis.value > -8000 && event.jaxis.value < 8000) break;
//
//                std::cout << "ControllerMovedEvent on axis " << event.jaxis.axis << " and value of  " << event.jaxis.value << std::endl;
//
//                ControllerMovedEvent controllerMovedEvent(event.jaxis.axis, event.jaxis.value);
//                m_data.EventCallback(controllerMovedEvent);
//
//                break;
//            }
//            case SDL_JOYBUTTONDOWN:
//            {
//                ControllerPressedEvent controllerPressedEvent(event.jbutton.button);
//                m_data.EventCallback(controllerPressedEvent);
//
//                break;
//            }
//            case SDL_JOYBUTTONUP:
//            {
//                ControllerReleasedEvent controllerReleasedEvent(event.jbutton.button);
//                m_data.EventCallback(controllerReleasedEvent);
//
//                break;
//            }
//            case SDL_JOYDEVICEADDED:
//            {
//                Input::AddController(event.cdevice.which);
//                ControllerAddedEvent controllerAddedEvent(event.cdevice.which);
//                m_data.EventCallback(controllerAddedEvent);
//
//                break;
//            }
//            case SDL_JOYDEVICEREMOVED:
//            {
//                Input::RemoveController(event.cdevice.which);
//                ControllerRemovedEvent controllerRemovedEvent(event.cdevice.which);
//                m_data.EventCallback(controllerRemovedEvent);
//                break;
//            }
//            //enable drag and drop in editor functionality
//#ifdef OO_EDITOR
//            case SDL_DROPBEGIN:
//            {
//                // event begin that triggers once on drop.
//                // triggers only once for multiple file drops.
//                FileDropEvent fileDrop(FileDropType::DropBegin, std::filesystem::path(), event.drop.windowID, event.drop.timestamp);
//                break;
//            }
//            case SDL_DROPFILE:
//            {
//                // SDL_DropFile actually allocates memory for the char* on this event.
//                // Triggers multiple times for multiple files.
//
//                FileDropEvent fileDrop(FileDropType::DropFile, event.drop.file, event.drop.windowID, event.drop.timestamp);
//                m_data.EventCallback(fileDrop);
//                // get our dir as an RAII string
//                std::string fileDir(event.drop.file);
//                SDL_free(event.drop.file);    // Free dropped_filedir memory
//
//                LOG_ENGINE_INFO("DroppedFile {0}, {1}", fileDir, (uint32_t)event.drop.timestamp);
//                // Shows directory of dropped file
//
//             //SDL_ShowSimpleMessageBox(
//             //    SDL_MESSAGEBOX_INFORMATION,
//             //    "File dropped on window",
//             //    fileDir.c_str(),
//             //    this->m_window
//             //);
//                break;
//            }
//#endif // OO_EDITOR
//
//            default:
//                break;
//            }
//        }
//
//        TRACY_PROFILE_SCOPE_END();
    }

    //void WindowsWindow::SwapBuffers()
    //{
    //    //TRACY_PROFILE_SCOPE("Swapping Buffers");

    //    // swap rendering buffers
    //    m_context->SwapBuffers();

    //    //TRACY_PROFILE_SCOPE_END();
    //}

    //void WindowsWindow::Maximize()
    //{
    //    SDL_MaximizeWindow(m_window);
    //}

    //void WindowsWindow::Minimize()
    //{
    //    SDL_MinimizeWindow(m_window);
    //}

    //void WindowsWindow::SetVSync(bool enabled)
    //{
    //    //LOG_ENGINE_INFO("Set Vsync : {0}", enabled);

    //    m_data.VSync = m_context->SetVSync(enabled);
    //}

    //void WindowsWindow::SetTitle(const std::string& title)
    //{
    //    m_data.Title = title;
    //    SDL_SetWindowTitle(m_window, m_data.Title.c_str());
    //}

    //void WindowsWindow::SetFullScreen(bool fullscreen)
    //{
    //    m_data.FullScreen = fullscreen;
    //    SDL_SetWindowFullscreen(m_window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);  // 0 means non full-screen
    //}

    //void WindowsWindow::ShowCursor(bool showCursor)
    //{
    //    showCursor ? SDL_ShowCursor(SDL_ENABLE) : SDL_ShowCursor(SDL_DISABLE);
    //}

    //std::pair<int, int> WindowsWindow::GetWindowPos() const
    //{
    //    int x, y;
    //    SDL_GetWindowPosition(m_window, &x, &y);
    //    return { x, y };
    //}

    inline bool WindowsWindow::IsVSync() const
    {
        return m_data.VSync;
    }

    inline bool WindowsWindow::IsFullscreen() const
    {
        return m_data.FullScreen;
    }

    inline bool WindowsWindow::IsFocused() const
    {
        return m_focused;
    }
}