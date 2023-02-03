/************************************************************************************//*!
\file           ColliderAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Nov 4, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to interact with the C++ Collider ECS Components (e.g. BoxCollider, SphereCollider, etc)

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Scripting/ScriptManager.h"

#include "Ouroboros/Scripting/ScriptValue.h"
#include "Ouroboros/Physics/ColliderComponents.h"

namespace oo
{
    /*-----------------------------------------------------------------------------*/
    /* SphereCollider                                                              */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API_GET_SET(SphereColliderComponent, Radius, float, Radius)
    SCRIPT_API_GET(SphereColliderComponent, GetGlobalRadius, float, GlobalRadius)

    /*-----------------------------------------------------------------------------*/
    /* BoxCollider                                                                 */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API void BoxCollider_GetSize(Scene::ID_type sceneID, UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        BoxColliderComponent& component = obj->GetComponent<BoxColliderComponent>();
        glm::vec3 vec3 = component.Size;
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void BoxCollider_SetSize(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        BoxColliderComponent& component = obj->GetComponent<BoxColliderComponent>();
        component.Size = { x, y, z };
    }

    SCRIPT_API void BoxCollider_GetHalfExtents(Scene::ID_type sceneID, UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        BoxColliderComponent& component = obj->GetComponent<BoxColliderComponent>();
        glm::vec3 vec3 = component.HalfExtents;
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void BoxCollider_SetHalfExtents(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        BoxColliderComponent& component = obj->GetComponent<BoxColliderComponent>();
        component.HalfExtents = { x, y, z };
    }

    SCRIPT_API void BoxCollider_GetGlobalHalfExtents(Scene::ID_type sceneID, UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        BoxColliderComponent& component = obj->GetComponent<BoxColliderComponent>();
        glm::vec3 vec3 = component.GlobalHalfExtents;
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    /*-----------------------------------------------------------------------------*/
    /* CapsuleCollider                                                             */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API_GET_SET(CapsuleColliderComponent, Radius, float, Radius)
    SCRIPT_API_GET_SET(CapsuleColliderComponent, HalfHeight, float, HalfHeight)
    SCRIPT_API_GET(CapsuleColliderComponent, GetGlobalRadius, float, GlobalRadius)
    SCRIPT_API_GET(CapsuleColliderComponent, GetGlobalHalfHeight, float, GlobalHalfHeight)

    /*-----------------------------------------------------------------------------*/
    /* ConvexCollider                                                              */
    /*-----------------------------------------------------------------------------*/
    MonoArray* ConvertVerticeArray(std::vector<vec3> vertices)
    {
        MonoClass* vecClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Vector3");
        MonoArray* arr = ScriptEngine::CreateArray(vecClass, vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            ScriptValue::vec3_type vertice{ vertices[i].x, vertices[i].y, vertices[i].z };
            mono_array_set(arr, ScriptValue::vec3_type, i, vertice);
        }
        return arr;
    }

    SCRIPT_API MonoArray* ConvexCollider_GetVertices(Scene::ID_type sceneID, UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        ConvexColliderComponent& component = obj->GetComponent<ConvexColliderComponent>();
        return ConvertVerticeArray(component.Vertices);
    }

    SCRIPT_API MonoArray* ConvexCollider_GetWorldVertices(Scene::ID_type sceneID, UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        ConvexColliderComponent& component = obj->GetComponent<ConvexColliderComponent>();
        return ConvertVerticeArray(component.WorldSpaceVertices);
    }
}