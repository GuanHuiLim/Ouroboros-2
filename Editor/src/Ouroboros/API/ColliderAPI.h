#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Scripting/ScriptManager.h"

#include "Ouroboros/Scripting/ScriptValue.h"
#include "Ouroboros/Physics/ColliderComponents.h"

namespace oo
{
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
    /* SphereCollider                                                              */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API_GET_SET(SphereColliderComponent, Radius, float, Radius)

    /*-----------------------------------------------------------------------------*/
    /* CapsuleCollider                                                             */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API_GET_SET(CapsuleColliderComponent, Radius, float, Radius)
    SCRIPT_API_GET_SET(CapsuleColliderComponent, HalfHeight, float, HalfHeight)

    /*-----------------------------------------------------------------------------*/
    /* ConvexCollider                                                              */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API MonoArray* ConvexCollider_GetVertices(Scene::ID_type sceneID, UUID uuid)
    {
        return nullptr;
    }

    SCRIPT_API MonoArray* ConvexCollider_GetWorldVertices(Scene::ID_type sceneID, UUID uuid)
    {
        return nullptr;
    }
}