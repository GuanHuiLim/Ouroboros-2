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
#include "Ouroboros/Scripting/ScriptValue.h"

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

#define SCRIPT_API_GET(Component, Name, Type, Variable) \
SCRIPT_API Type Component##_##Name(Scene::ID_type sceneID, UUID uuid) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    return component.Variable; \
}

#define SCRIPT_API_SET(Component, Name, Type, Variable) \
SCRIPT_API void Component##_##Name(Scene::ID_type sceneID, UUID uuid, Type value) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    component.Variable = value; \
}

#define SCRIPT_API_SET_A(Component, Name, Type, SetFunction, Additional) \
SCRIPT_API void Component##_##Name(Scene::ID_type sceneID, UUID uuid, Type value) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    component.Variable = value; \
    Additional \
}

#define SCRIPT_API_GET_SET(Component, Name, Type, Variable) \
SCRIPT_API_GET(Component, Get##Name, Type, Variable) \
SCRIPT_API_SET(Component, Set##Name, Type, Variable)

#define SCRIPT_API_GET_SET_A(Component, Name, Type, Variable, Additional) \
SCRIPT_API_GET(Component, Get##Name, Type, Variable) \
SCRIPT_API_SET_A(Component, Set##Name, Type, Variable, Additional)

#define SCRIPT_API_GET_FUNC(Component, Name, Type, GetFunction) \
SCRIPT_API Type Component##_##Name(Scene::ID_type sceneID, UUID uuid) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    return component.GetFunction(); \
}

#define SCRIPT_API_SET_FUNC(Component, Name, Type, SetFunction) \
SCRIPT_API void Component##_##Name(Scene::ID_type sceneID, UUID uuid, Type value) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    component.SetFunction(value); \
}

#define SCRIPT_API_SET_FUNC_A(Component, Name, Type, SetFunction, Additional) \
SCRIPT_API void Component##_##Name(Scene::ID_type sceneID, UUID uuid, Type value) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    component.SetFunction(value); \
    Additional \
}

#define SCRIPT_API_GET_SET_FUNC(Component, Name, Type, GetFunction, SetFunction) \
SCRIPT_API_GET_FUNC(Component, Get##Name, Type, GetFunction) \
SCRIPT_API_SET_FUNC(Component, Set##Name, Type, SetFunction)

#define SCRIPT_API_GET_SET_FUNC_A(Component, Name, Type, GetFunction, SetFunction, Additional) \
SCRIPT_API_GET_FUNC(Component, Get##Name, Type, GetFunction) \
SCRIPT_API_SET_FUNC_A(Component, Set##Name, Type, SetFunction, Additional)

#pragma endregion

#pragma region Vector3

// C# DLLImport code for Getting a Vector3
/*
[DllImport("__Internal")] private static extern Vector3 { Component }_{ Name }(uint sceneID, ulong instanceID);
*/
// C# DLLImport code for Setting a Vector3
/*
[DllImport("__Internal")] private static extern void { Component }_{ Name }(uint sceneID, ulong instanceID, Vector3 value);
*/

#define SCRIPT_API_GET_VECTOR3(Component, Name, Variable) \
SCRIPT_API ScriptValue::vec3_type Component##_##Name(Scene::ID_type sceneID, UUID uuid) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    return ScriptValue::vec3_type{ component.Variable.x, component.Variable.y, component.Variable.z };\
}

#define SCRIPT_API_SET_VECTOR3(Component, Name, Variable) \
SCRIPT_API void Component##_##Name(Scene::ID_type sceneID, UUID uuid, ScriptValue::vec3_type value) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    component.Variable = { value.x, value.y, value.z }; \
}

#define SCRIPT_API_GET_SET_VECTOR3(Component, Name, Variable) \
SCRIPT_API_GET_VECTOR3(Component, Get##Name, Variable) \
SCRIPT_API_SET_VECTOR3(Component, Set##Name, Variable)

#define SCRIPT_API_GET_FUNC_VECTOR3(Component, Name, GetFunction) \
SCRIPT_API ScriptValue::vec3_type Component##_##Name(Scene::ID_type sceneID, UUID uuid) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    glm::vec3 vec = component.GetFunction();\
    return ScriptValue::vec3_type{ vec.x, vec.y, vec.z };\
}

#define SCRIPT_API_SET_FUNC_VECTOR3(Component, Name, SetFunction) \
SCRIPT_API void Component##_##Name(Scene::ID_type sceneID, UUID uuid, ScriptValue::vec3_type value) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    component.SetFunction({ value.x, value.y, value.z }); \
}

#define SCRIPT_API_GET_SET_FUNC_VECTOR3(Component, Name, GetFunction, SetFunction) \
SCRIPT_API_GET_FUNC_VECTOR3(Component, Get##Name, GetFunction) \
SCRIPT_API_SET_FUNC_VECTOR3(Component, Set##Name, SetFunction)

#pragma endregion

#pragma region Vector2

// C# DLLImport code for Getting a Vector2
/*
[DllImport("__Internal")] private static extern Vector2 { Component }_{ Name }(uint sceneID, ulong instanceID);
*/
// C# DLLImport code for Setting a Vector2
/*
[DllImport("__Internal")] private static extern void { Component }_{ Name }(uint sceneID, ulong instanceID, Vector2 value);
*/

#define SCRIPT_API_GET_VECTOR2(Component, Name, Variable) \
SCRIPT_API ScriptValue::vec2_type Component##_##Name(Scene::ID_type sceneID, UUID uuid) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    return ScriptValue::vec2_type{ component.Variable.x, component.Variable.y };\
}

#define SCRIPT_API_SET_VECTOR2(Component, Name, Variable) \
SCRIPT_API void Component##_##Name(Scene::ID_type sceneID, UUID uuid, ScriptValue::vec2_type value) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    component.Variable = { value.x, value.y }; \
}

#define SCRIPT_API_GET_SET_VECTOR2(Component, Name, Variable) \
SCRIPT_API_GET_VECTOR2(Component, Get##Name, Variable) \
SCRIPT_API_SET_VECTOR2(Component, Set##Name, Variable)

#define SCRIPT_API_GET_FUNC_VECTOR2(Component, Name, GetFunction) \
SCRIPT_API ScriptValue::vec2_type Component##_##Name(Scene::ID_type sceneID, UUID uuid) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    glm::vec2 vec = component.GetFunction();\
    return ScriptValue::vec2_type{ vec.x, vec.y };\
}

#define SCRIPT_API_SET_FUNC_VECTOR2(Component, Name, SetFunction) \
SCRIPT_API void Component##_##Name(Scene::ID_type sceneID, UUID uuid, ScriptValue::vec2_type value) \
{ \
    std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid); \
    Component& component = obj->GetComponent<Component>(); \
    component.SetFunction({ value.x, value.y }); \
}

#define SCRIPT_API_GET_SET_FUNC_VECTOR2(Component, Name, GetFunction, SetFunction) \
SCRIPT_API_GET_FUNC_VECTOR2(Component, Get##Name, GetFunction) \
SCRIPT_API_SET_FUNC_VECTOR2(Component, Set##Name, SetFunction)

#pragma endregion
