/************************************************************************************//*!
\file           ScriptComponent.h
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

/*-----------------------------------------------------------------------------*/
/* C# Export Function Guide                                                    */
/*-----------------------------------------------------------------------------*/
// If you need to write your own export function, copy paste this, and replace any bracketed { stuff } accordingly
/*
SCRIPT_API { Return Type } { Function Name }(Entity instanceID, { Other Parameters })
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
[DllImport("__Internal")] private static extern { Return Type } {Function Name}(int instanceID, { Other Parameters });
*/
// Common types from C++ to C#
/*
const char* -> string
{ insert type here }* -> out { insert type here } (e.g. float* -> out float)
*/

#define SCRIPT_API extern "C" __declspec(dllexport)

#pragma region Vector3

// C# DLLImport code for Getting Vector3
/*
[DllImport("__Internal")] private static extern void { Component }_{ Name }(int instanceID, out float x, out float y, out float z);
*/
// C# DLLImport code for Setting Vector3
/*
[DllImport("__Internal")] private static extern void{ Component }_{ Name }(int instanceID, float x, float y, float z);
*/

#define SCRIPT_API_VECTOR3_GET(Component, Name, Variable) \
SCRIPT_API void Component##_##Name(Entity instanceID, float* x, float* y, float* z) \
{ \
    GameObject obj{ instanceID }; \
    if (!GameObject::IsValid(obj)) \
    { \
        ScriptEngine::ThrowNullException(); \
    } \
    Component& component = obj.GetComponent<Component>(); \
    oom::vec3 vector3 = component.Variable(); \
    *x = vector3.x; \
    *y = vector3.y; \
    *z = vector3.z; \
}

#define SCRIPT_API_VECTOR3_SET(Component, Name, Variable) \
SCRIPT_API void Component##_##Name(Entity instanceID, float x, float y, float z) \
{ \
    GameObject obj{ instanceID }; \
    if (!GameObject::IsValid(obj)) \
    { \
        ScriptEngine::ThrowNullException(); \
    } \
    Component& component = obj.GetComponent<Component>(); \
    component.Variable({ x, y, z }); \
}

#define SCRIPT_API_VECTOR3_SET_A(Component, Name, Variable, Additional) \
SCRIPT_API void Component##_##Name(Entity instanceID, float x, float y, float z) \
{ \
    GameObject obj{ instanceID }; \
    if (!GameObject::IsValid(obj)) \
    { \
        ScriptEngine::ThrowNullException(); \
    } \
    Component& component = obj.GetComponent<Component>(); \
    component.Variable({ x, y, z }); \
    Additional; \
}

#define SCRIPT_API_VECTOR3_GET_SET(Component, Name, GetVariable, SetVariable) \
SCRIPT_API_VECTOR3_GET(Component, Get##Name, GetVariable) \
SCRIPT_API_VECTOR3_SET(Component, Set##Name, SetVariable)

#define SCRIPT_API_VECTOR3_GET_SET_A(Component, Name, GetVariable, SetVariable, Additional) \
SCRIPT_API_VECTOR3_GET(Component, Get##Name, GetVariable) \
SCRIPT_API_VECTOR3_SET_A(Component, Set##Name, SetVariable, Additional)

#pragma endregion

#pragma region float

// C# DLLImport code for Getting float
/*
[DllImport("__Internal")] private static extern float { Component }_{ Name }(int instanceID);
*/
// C# DLLImport code for Setting float
/*
[DllImport("__Internal")] private static extern void{ Component }_{ Name }(int instanceID, float value);
*/

#define SCRIPT_API_FLOAT_GET(Component, Name, Variable) \
SCRIPT_API float Component##_##Name(Entity instanceID) \
{ \
    GameObject obj{ instanceID }; \
    if (!GameObject::IsValid(obj)) \
    { \
        ScriptEngine::ThrowNullException(); \
    } \
    Component& component = obj.GetComponent<Component>(); \
    return component.Variable(); \
}

#define SCRIPT_API_FLOAT_SET(Component, Name, Variable) \
SCRIPT_API void Component##_##Name(Entity instanceID, float value) \
{ \
    GameObject obj{ instanceID }; \
    if (!GameObject::IsValid(obj)) \
    { \
        ScriptEngine::ThrowNullException(); \
    } \
    Component& component = obj.GetComponent<Component>(); \
    component.Variable(value); \
}

#define SCRIPT_API_FLOAT_SET_A(Component, Name, Variable, Additional) \
SCRIPT_API void Component##_##Name(Entity instanceID, float value) \
{ \
    GameObject obj{ instanceID }; \
    if (!GameObject::IsValid(obj)) \
    { \
        ScriptEngine::ThrowNullException(); \
    } \
    Component& component = obj.GetComponent<Component>(); \
    component.Variable(value); \
    Additional; \
}

#define SCRIPT_API_FLOAT_GET_SET(Component, Name, GetVariable, SetVariable) \
SCRIPT_API_FLOAT_GET(Component, Get##Name, GetVariable) \
SCRIPT_API_FLOAT_SET(Component, Set##Name, SetVariable)

#define SCRIPT_API_FLOAT_GET_SET_A(Component, Name, GetVariable, SetVariable, Additional) \
SCRIPT_API_FLOAT_GET(Component, Get##Name, GetVariable) \
SCRIPT_API_FLOAT_SET(Component, Set##Name, SetVariable, Additional)

#pragma endregion
