#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Scripting/ScriptManager.h"

#include "Ouroboros/Scripting/ScriptValue.h"
#include "Ouroboros/Physics/RigidbodyComponent.h"

namespace oo
{
    SCRIPT_API void Rigidbody_GetVelocity(Scene::ID_type sceneID, UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        RigidbodyComponent& component = obj->GetComponent<RigidbodyComponent>();
        glm::vec3 vec3 = component.GetLinearVelocity();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Rigidbody_SetVelocity(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        RigidbodyComponent& component = obj->GetComponent<RigidbodyComponent>();
        component.SetLinearVelocity({ x, y, z });
    }

    SCRIPT_API void Rigidbody_AddForce(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        RigidbodyComponent& component = obj->GetComponent<RigidbodyComponent>();
        component.AddForce(glm::vec3{ x, y, z });
    }
}