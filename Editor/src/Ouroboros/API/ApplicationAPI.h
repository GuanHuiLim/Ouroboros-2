/************************************************************************************//*!
\file           ApplicationAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to perform actions related to the application in general, like 
                getting the screen size, setting the visibility of the cursor,
                and quitting the application

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"

#include "Ouroboros/Core/Application.h"
#include "Ouroboros/Core/WindowsWindow.h"

#include "Ouroboros/Core/Timer.h"

#include "Project.h"

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
        Application::Get().GetWindow().ShowCursor(isVisible);
    }

    SCRIPT_API void Cursor_SetLocked(bool isLocked)
    {
        Application::Get().GetWindow().SetMouseLockState(isLocked);
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
    SCRIPT_API float Time_GetTimeScale()
    {
        // return static_cast<float>(Timestep::TimeScale);
        return timer::get_timescale();
    }

    SCRIPT_API void Time_SetTimeScale(float timeScale)
    {
        // Timestep::TimeScale = timeScale;
        timer::set_timescale(timeScale);
    }

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
    /* Asset Functions for C#                                                      */
    /*-----------------------------------------------------------------------------*/
    
    SCRIPT_API AssetID Asset_LoadAssetAtPath(const char* path)
    {
        Asset asset = Project::GetAssetManager()->LoadPath(path);
        return asset.GetID();
    }

    SCRIPT_API ScriptDatabase::IntPtr Asset_GetName(AssetID assetID)
    {
        Asset asset = Project::GetAssetManager()->Get(assetID);
        if (asset.GetID() == Asset::ID_NULL)
            ScriptEngine::ThrowNullException();

        std::string const& name = asset.GetFilePath().filename().string();
        MonoString* string = ScriptEngine::CreateString(name.c_str());
        return mono_gchandle_new((MonoObject*)string, false);
    }

    SCRIPT_API unsigned int Asset_GetType(AssetID assetID)
    {
        Asset asset = Project::GetAssetManager()->Get(assetID);
        if (asset.GetID() == Asset::ID_NULL)
            ScriptEngine::ThrowNullException();

        return static_cast<unsigned int>(asset.GetType());
    }

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