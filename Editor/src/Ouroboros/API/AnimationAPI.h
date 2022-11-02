/************************************************************************************//*!
\file           AnimationAPI.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief
Defines the exported helper functions that the C# scripts will use
                to interact with the C++ Animation System/Component

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"

#include "Ouroboros/Scripting/ScriptSystem.h"
#include "Ouroboros/Animation/AnimationComponent.h"

namespace oo
{
    SCRIPT_API size_t AnimationComponent_GetParameterID(Scene::ID_type sceneID, oo::UUID uuid, const char* paramName)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        return component.GetParameterID(paramName);
    }

    SCRIPT_API uint AnimationComponent_GetParameterIndex(Scene::ID_type sceneID, oo::UUID uuid, const char* paramName)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        return component.GetParameterIndex(paramName);
    }
    /*---------------
    INT
    ---------------*/
    SCRIPT_API void AnimationComponent_SetParameterByName_int(Scene::ID_type sceneID, oo::UUID uuid, const char* paramName, int val )
    { 
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        component.SetParameter(paramName, val);
    }

    SCRIPT_API void AnimationComponent_SetParameterByID_int(Scene::ID_type sceneID, oo::UUID uuid, size_t id, int val)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        component.SetParameterByID(id, val);
    }

    SCRIPT_API void AnimationComponent_SetParameterByIndex_int(Scene::ID_type sceneID, oo::UUID uuid, uint index, int val)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        component.SetParameterByIndex(index, val);
    }
    /*---------------
    FLOAT
    ---------------*/
    SCRIPT_API void AnimationComponent_SetParameterByName_float(Scene::ID_type sceneID, oo::UUID uuid, const char* paramName, float val)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        component.SetParameter(paramName, val);
    }

    SCRIPT_API void AnimationComponent_SetParameterByID_float(Scene::ID_type sceneID, oo::UUID uuid, size_t id, float val)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        component.SetParameterByID(id, val);
    }

    SCRIPT_API void AnimationComponent_SetParameterByIndex_float(Scene::ID_type sceneID, oo::UUID uuid, uint index, float val)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        component.SetParameterByIndex(index, val);
    }
    /*---------------
    BOOL
    ---------------*/
    SCRIPT_API void AnimationComponent_SetParameterByName_bool(Scene::ID_type sceneID, oo::UUID uuid, const char* paramName, bool val)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        component.SetParameter(paramName, val);
    }

    SCRIPT_API void AnimationComponent_SetParameterByID_bool(Scene::ID_type sceneID, oo::UUID uuid, size_t id, bool val)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        component.SetParameterByID(id, val);
    }

    SCRIPT_API void AnimationComponent_SetParameterByIndex_bool(Scene::ID_type sceneID, oo::UUID uuid, uint index, bool val)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        AnimationComponent& component = obj->GetComponent<AnimationComponent>();
        component.SetParameterByIndex(index, val);
    }
}