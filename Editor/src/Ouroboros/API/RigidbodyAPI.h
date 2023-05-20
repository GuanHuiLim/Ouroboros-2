/************************************************************************************//*!
\file           RigidbodyAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Nov 4, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to interact with the C++ RigidbodyComponent ECS Component

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Scripting/ScriptManager.h"

#include "Ouroboros/Scripting/ScriptValue.h"
#include "Ouroboros/Physics/RigidbodyComponent.h"

namespace oo
{
    SCRIPT_API_GET_SET_FUNC(RigidbodyComponent, IsStatic, bool, IsStatic, SetStatic)

    SCRIPT_API_GET_SET_FUNC(RigidbodyComponent, IsTrigger, bool, IsTrigger, SetTrigger)

    SCRIPT_API_GET_SET_FUNC(RigidbodyComponent, Gravity, bool, IsGravityEnabled, SetGravity)

    SCRIPT_API_GET_SET_FUNC(RigidbodyComponent, Mass, float, GetMass, SetMass)
    SCRIPT_API_GET_SET_FUNC(RigidbodyComponent, LinearDamping, float, GetLinearDamping, SetLinearDamping)
    SCRIPT_API_GET_SET_FUNC(RigidbodyComponent, AngularDamping, float, GetAngularDamping, SetAngularDamping)

    SCRIPT_API_GET_SET_FUNC(RigidbodyComponent, InputLayer, uint32_t, GetInputLayer, SetInputLayer)
    SCRIPT_API_GET_SET_FUNC(RigidbodyComponent, OutputLayer, uint32_t, GetOutputLayer, SetOutputLayer)

    SCRIPT_API void RigidbodyComponent_GetOffset(Scene::ID_type sceneID, UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        RigidbodyComponent& component = obj->GetComponent<RigidbodyComponent>();
        glm::vec3 vec3 = component.Offset;
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void RigidbodyComponent_SetOffset(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        RigidbodyComponent& component = obj->GetComponent<RigidbodyComponent>();
        component.Offset = glm::vec3({ x, y, z });
    }

    SCRIPT_API_GET_SET(RigidbodyComponent, LockXAxisPosition, bool, LockXAxisPosition)
    SCRIPT_API_GET_SET(RigidbodyComponent, LockYAxisPosition, bool, LockYAxisPosition)
    SCRIPT_API_GET_SET(RigidbodyComponent, LockZAxisPosition, bool, LockZAxisPosition)
    SCRIPT_API_GET_SET(RigidbodyComponent, LockXAxisRotation, bool, LockXAxisRotation)
    SCRIPT_API_GET_SET(RigidbodyComponent, LockYAxisRotation, bool, LockYAxisRotation)
    SCRIPT_API_GET_SET(RigidbodyComponent, LockZAxisRotation, bool, LockZAxisRotation)

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

    SCRIPT_API void Rigidbody_AddForceWithMode(Scene::ID_type sceneID, UUID uuid, float x, float y, float z, int forceMode)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        RigidbodyComponent& component = obj->GetComponent<RigidbodyComponent>();
        component.AddForce(glm::vec3{ x, y, z }, static_cast<ForceMode>(forceMode));
    }

    SCRIPT_API void Rigidbody_AddTorque(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        RigidbodyComponent& component = obj->GetComponent<RigidbodyComponent>();
        component.AddTorque(glm::vec3{ x, y, z });
    }

    SCRIPT_API void Rigidbody_AddTorqueWithMode(Scene::ID_type sceneID, UUID uuid, float x, float y, float z, int forceMode)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        RigidbodyComponent& component = obj->GetComponent<RigidbodyComponent>();
        component.AddTorque(glm::vec3{ x, y, z }, static_cast<ForceMode>(forceMode));
    }
}