#pragma once
#include "Scripting/ExportAPI.h"

#include "Ouroboros/Core/Application.h"
#include "Ouroboros/Core/WindowsWindow.h"

#include "Ouroboros/Core/Timer.h"

#include "Ouroboros/Core/Log.h"

namespace oo
{
    /*-----------------------------------------------------------------------------*/
    /* Application Functions for C#                                                */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API void Application_Quit()
    {
        #if OO_EXECUTABLE
            Application::Get().Close();
        #elif OO_EDITOR
            //GenericButtonEvents e = { Buttons::STOP_BUTTON };
            //oo::EventManager::Broadcast(&e);
        #endif
    }

    /*-----------------------------------------------------------------------------*/
    /* Cursor Functions for C#                                                     */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API void Cursor_SetVisible(bool isVisible)
    {
        reinterpret_cast<WindowsWindow&>(Application::Get().GetWindow()).ShowCursor(isVisible);
    }

    /*-----------------------------------------------------------------------------*/
    /* Screen Functions for C#                                                     */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API unsigned int Screen_GetWidth()
    {
        return Application::Get().GetWindow().GetWidth();
    }

    SCRIPT_API unsigned int Screen_GetHeight()
    {
        return Application::Get().GetWindow().GetHeight();
    }

    SCRIPT_API bool Screen_GetFullScreen()
    {
        return Application::Get().GetWindow().IsFullscreen();
    }

    SCRIPT_API void Screen_SetFullScreen(bool fullscreen)
    {
        Application::Get().GetWindow().SetFullScreen(fullscreen);
    }

    /*-----------------------------------------------------------------------------*/
    /* Time Functions for C#                                                       */
    /*-----------------------------------------------------------------------------*/
    //SCRIPT_API float Time_GetTimeScale()
    //{
    //    return static_cast<float>(Timestep::TimeScale);
    //}

    //SCRIPT_API void Time_SetTimeScale(float timeScale)
    //{
    //    Timestep::TimeScale = timeScale;
    //}

    SCRIPT_API float Time_GetDeltaTime()
    {
        // return Timestep::DeltaTime();
        return timer::dt();
    }

    SCRIPT_API float Time_GetUnscaledDeltaTime()
    {
        // return Timestep::UnscaledTime();
        return timer::unscaled_dt();
    }

    //SCRIPT_API float Time_GetFixedDeltaTime()
    //{
    //    return static_cast<float>(PhysicsSystem::FixedDeltaTime * Timestep::TimeScale);
    //}

    //SCRIPT_API float Time_GetFixedUnscaledDeltaTime()
    //{
    //    return static_cast<float>(PhysicsSystem::FixedDeltaTime);
    //}

    /*-----------------------------------------------------------------------------*/
    /* Debug Functions for C#                                                      */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API void Log(const char* msg)
    {
        LOG_TRACE(msg);
    }

    SCRIPT_API void LogInfo(const char* msg)
    {
        LOG_INFO(msg);
    }

    SCRIPT_API void LogWarning(const char* msg)
    {
        LOG_WARN(msg);
    }

    SCRIPT_API void LogError(const char* msg)
    {
        LOG_ERROR(msg);
    }

    SCRIPT_API void LogCritical(const char* msg)
    {
        LOG_CRITICAL(msg);
    }
}