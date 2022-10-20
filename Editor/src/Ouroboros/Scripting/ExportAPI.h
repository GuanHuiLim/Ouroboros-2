/************************************************************************************//*!
\file           ExportAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Declares the macros used to make exposing C++ functions
                for C# scripts to use easier

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/Scripting/ScriptManager.h"

/*-----------------------------------------------------------------------------*/
/* C# Export Function Guide                                                    */
/*-----------------------------------------------------------------------------*/
// If you need to write your own export function, copy paste this, and replace any bracketed { stuff } accordingly
/*
SCRIPT_API { Return Type } { Function Name }(Scene::ID_type sceneID, UUID uuid, { Other Parameters })
{
}
*/

// If you need to get a scene from an ID, or gameObject from scene, make use of
// ScriptManager::GetScene
// ScriptManager::GetObjectFromScene
// They are special wrapper functions that will throw a C# null reference if the scene/GameObject does not exist

// Even using helper macros, you'll need to write the C# code on your own. Just copy paste this in the corresponding C# class
// Replace any bracketed { stuff } with the same name as the C++ code
/*
[DllImport("__Internal")] private static extern { Return Type } { Function Name }(uint sceneID, ulong instanceID, { Other Parameters });
*/
// Common types from C++ to C#
/*
const char* -> string
{ insert type here }* -> out { insert type here } (e.g. float* -> out float)
*/

#define SCRIPT_API extern "C" __declspec(dllexport)

#pragma region General

// Check to make sure if specific macros are provided for a given type. If there are, use them instead of the general macros

// C# DLLImport code for a simple function (replace text in { })
/*
[DllImport("__Internal")] private static extern void { Component }_{ Function }(uint sceneID, ulong instanceID);
*/

#define SCRIPT_API_FUNCTION(Component, Function) \
SCRIPT_API void Component##_##Function(Scene::ID_type sceneID, UUID uuid) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    obj->GetComponent<Component>().Function(); \
}

// C# DLLImport code for Getting a variable (replace text in { })
/*
[DllImport("__Internal")] private static extern { Type } { Component }_{ Name }(uint sceneID, ulong instanceID);
*/
// C# DLLImport code for Setting a variable (replace text in { })
/*
[DllImport("__Internal")] private static extern void { Component }_{ Name }(uint sceneID, ulong instanceID, { Type } value);
*/

#define SCRIPT_API_GET(Component, Name, Type, GetFunction) \
SCRIPT_API Type Component##_##Name(Scene::ID_type sceneID, UUID uuid) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    return component.GetFunction(); \
}

#define SCRIPT_API_SET(Component, Name, Type, SetFunction) \
SCRIPT_API void Component##_##Name(Scene::ID_type sceneID, UUID uuid, Type value) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    component.SetFunction(value); \
}

#define SCRIPT_API_SET_A(Component, Name, Type, SetFunction, Additional) \
SCRIPT_API void Component##_##Name(Scene::ID_type sceneID, UUID uuid, Type value) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    component.SetFunction(value); \
    Additional \
}

#define SCRIPT_API_GET_SET(Component, Name, Type, GetFunction, SetFunction) \
SCRIPT_API_GET(Component, Get##Name, Type, GetFunction) \
SCRIPT_API_SET(Component, Set##Name, Type, SetFunction)

#define SCRIPT_API_GET_SET_A(Component, Name, Type, GetFunction, SetFunction, Additional) \
SCRIPT_API_GET(Component, Get##Name, GetFunction) \
SCRIPT_API_SET(Component, Set##Name, SetFunction, Additional)

#pragma endregion