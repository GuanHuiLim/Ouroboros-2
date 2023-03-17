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
#include "Ouroboros/Physics/PhysicsSystem.h"

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

    SCRIPT_API ScriptDatabase::IntPtr Application_GetAssetPath()
    {
        MonoString* str = ScriptEngine::CreateString(Project::GetAssetFolder().string().c_str());
        return mono_gchandle_new((MonoObject*)str, false);
    }

    /*-----------------------------------------------------------------------------*/
    /* Cursor Functions for C#                                                     */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API void Cursor_SetVisible(bool isVisible)
    {
        Application::Get().GetWindow().ShowCursor(isVisible);
    }

    SCRIPT_API bool Cursor_GetLocked()
    {
        return Application::Get().GetWindow().GetMouseCursorMode();
    }

    SCRIPT_API void Cursor_SetLocked(bool isLocked)
    {
        Application::Get().GetWindow().SetMouseLockState(isLocked);
    }

    SCRIPT_API void Cursor_SetPosition(int x, int y)
    {
        Application::Get().GetWindow().SetCursorPosition(x, y);
    }

    /*-----------------------------------------------------------------------------*/
    /* Screen Functions for C#                                                     */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API int Screen_GetWidth()
    {
        return Application::Get().GetWindow().GetWidth();
    }

    SCRIPT_API int Screen_GetHeight()
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
        return timer::get_timescale();
    }

    SCRIPT_API void Time_SetTimeScale(float timeScale)
    {
        timer::set_timescale(timeScale);
    }

    SCRIPT_API float Time_GetDeltaTime()
    {
        return timer::dt();
    }

    SCRIPT_API float Time_GetUnscaledDeltaTime()
    {
        return timer::unscaled_dt();
    }

    SCRIPT_API float Time_GetFixedDeltaTime()
    {
        return static_cast<float>(PhysicsSystem::GetFixedDeltaTime());
    }

    SCRIPT_API float Time_GetFixedTimeScale()
    {
        return PhysicsSystem::GetFixedDeltaTimescale();
    }

    SCRIPT_API void Time_SetFixedTimeScale(float value)
    {
        PhysicsSystem::SetFixedDeltaTimescale(value);
    }

    /*-----------------------------------------------------------------------------*/
    /* Audio Functions for C#                                                       */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API float Audio_GetGroupVolume(int group)
    {
        float value;
        FMOD_ERR_HAND(audio::GetChannelGroup(static_cast<AudioSourceGroup>(group))->getVolume(&value));
        return value;
    }

    SCRIPT_API void Audio_SetGroupVolume(int group, float value)
    {
        FMOD_ERR_HAND(audio::GetChannelGroup(static_cast<AudioSourceGroup>(group))->setVolume(value));
    }

    /*-----------------------------------------------------------------------------*/
    /* Asset Functions for C#                                                      */
    /*-----------------------------------------------------------------------------*/
    
    SCRIPT_API AssetID Asset_LoadAssetAtPath(const char* path)
    {
        Asset asset = Project::GetAssetManager()->GetOrLoadPath(path);
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

    SCRIPT_API MonoArray* Asset_GetByType(AssetInfo::Type assetType, const char* name_space, const char* name)
    {
        std::vector<oo::Asset> assetList = Project::GetAssetManager()->GetAssetsByType(assetType);

        MonoClass* baseClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Asset");
        MonoClassField* field = mono_class_get_field_from_name(baseClass, "id");

        MonoClass* klass = ScriptEngine::GetClass("ScriptCore", name_space, name);
        MonoArray* arr = ScriptEngine::CreateArray(klass, assetList.size());

        for (int i = 0; i < assetList.size(); ++i)
        {
            MonoObject* asset = ScriptEngine::CreateObject(klass);
            AssetID id = assetList[i].GetID();
            mono_field_set_value(asset, field, &id);

            mono_array_set(arr, MonoObject*, i, asset);
        }
        return arr;
    }
}