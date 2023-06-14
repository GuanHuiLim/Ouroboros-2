/************************************************************************************//*!
\file           PhysicsAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Nov 4, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to interact with the C++ Physics System in the ECS Systems

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Scripting/ScriptManager.h"
#include "Ouroboros/Scripting/ScriptValue.h"

#include "Ouroboros/Physics/PhysicsSystem.h"
#include "Ouroboros/Physics/RigidbodyComponent.h"

namespace oo
{
    // Helper Functions to create a C# RaycastHit from a C++ RaycastResult
    void FillRaycastHit(RaycastResult& result, MonoObject* hitInfo)
    {
        ScriptSystem* ss = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<ScriptSystem>();
        MonoClass* dataClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "RaycastHit");

        MonoClassField* field = mono_class_get_field_from_name(dataClass, "transform");
        ComponentDatabase::IntPtr ptr = ss->GetComponent(result.UUID, "Ouroboros", "Transform");
        if (ptr == ComponentDatabase::InvalidPtr)
            mono_field_set_value(hitInfo, field, nullptr);
        else
            mono_field_set_value(hitInfo, field, mono_gchandle_get_target(ptr));

        field = mono_class_get_field_from_name(dataClass, "rigidbody");
        ptr = ss->GetComponent(result.UUID, "Ouroboros", "Rigidbody");
        if (ptr == ComponentDatabase::InvalidPtr)
            mono_field_set_value(hitInfo, field, nullptr);
        else
            mono_field_set_value(hitInfo, field, mono_gchandle_get_target(ptr));

        field = mono_class_get_field_from_name(dataClass, "collider");
        ptr = ss->GetComponent(result.UUID, "Ouroboros", "Collider");
        if (ptr == ComponentDatabase::InvalidPtr)
            mono_field_set_value(hitInfo, field, nullptr);
        else
            mono_field_set_value(hitInfo, field, mono_gchandle_get_target(ptr));

        field = mono_class_get_field_from_name(dataClass, "point");
        ScriptValue fieldValue = ScriptValue{ ScriptValue::vec3_type{ result.Position.x, result.Position.y, result.Position.z } };
        ScriptValue::SetFieldValue(hitInfo, field, fieldValue);

        field = mono_class_get_field_from_name(dataClass, "normal");
        fieldValue = ScriptValue{ ScriptValue::vec3_type{ result.Normal.x, result.Normal.y, result.Normal.z } };
        ScriptValue::SetFieldValue(hitInfo, field, fieldValue);

        field = mono_class_get_field_from_name(dataClass, "distance");
        mono_field_set_value(hitInfo, field, &(result.Distance));
    }

    MonoObject* CreateRaycastHit(RaycastResult& result)
    {
        MonoClass* dataClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "RaycastHit");
        MonoObject* hitInfo = ScriptEngine::CreateObject(dataClass);
        FillRaycastHit(result, hitInfo);
        return hitInfo;
    }

    MonoArray* CreateRaycastHitArray(std::vector<RaycastResult> resultList)
    {
        ScriptSystem* ss = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<ScriptSystem>();
        MonoClass* dataClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "RaycastHit");

        MonoArray* arr = ScriptEngine::CreateArray(dataClass, resultList.size());
        int32_t size = mono_class_array_element_size(dataClass);
        for (size_t i = 0; i < resultList.size(); ++i)
        {
            char* element = reinterpret_cast<char*>(mono_array_addr_with_size(arr, size, i));
            RaycastResult& result = resultList[i];
            
            MonoObject** setter = reinterpret_cast<MonoObject**>(element);
            ComponentDatabase::IntPtr ptr = ss->GetComponent(result.UUID, "Ouroboros", "Transform");
            if (ptr == ComponentDatabase::InvalidPtr)
                *setter = nullptr;
            else
                *setter = mono_gchandle_get_target(ptr);

            element += sizeof(MonoObject*);
            setter = reinterpret_cast<MonoObject**>(element);
            ptr = ss->GetComponent(result.UUID, "Ouroboros", "Rigidbody");
            if (ptr == ComponentDatabase::InvalidPtr)
                *setter = nullptr;
            else
                *setter = mono_gchandle_get_target(ptr);

            element += sizeof(MonoObject*);
            setter = reinterpret_cast<MonoObject**>(element);
            ptr = ss->GetComponent(result.UUID, "Ouroboros", "Collider");
            if (ptr == ComponentDatabase::InvalidPtr)
                *setter = nullptr;
            else
                *setter = mono_gchandle_get_target(ptr);

            element += sizeof(MonoObject*);
            ScriptValue::vec3_type* vecSetter = reinterpret_cast<ScriptValue::vec3_type*>(element);
            *vecSetter = ScriptValue::vec3_type{ result.Position.x, result.Position.y, result.Position.z };

            element += sizeof(float) * 3;
            vecSetter = reinterpret_cast<ScriptValue::vec3_type*>(element);
            *vecSetter = ScriptValue::vec3_type{ result.Normal.x, result.Normal.y, result.Normal.z };

            element += sizeof(float) * 3;
            float* floatSetter = reinterpret_cast<float*>(element);
            *floatSetter = result.Distance;
;        }
        return arr;
    }

    SCRIPT_API LayerType GenerateCollisionMask(const char* strArray[], int size)
    {
        size_t u_size = static_cast<size_t>(size);
        std::vector<std::string> layerNames{ static_cast<size_t>(u_size) };
        for (size_t i = 0; i < u_size; ++i)
        {
            layerNames.emplace_back(strArray[i]);
        }
        return PhysicsSystem::GenerateCollisionMask(layerNames);
    }

    SCRIPT_API bool Physics_RaycastBasic(ScriptValue::vec3_type origin, ScriptValue::vec3_type dir)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        oo::Ray ray{ { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };
        
        RaycastResult result;
        try
        {
            result = ps->Raycast(ray);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        return result.Intersect;
    }

    SCRIPT_API bool Physics_Raycast(ScriptValue::vec3_type origin, ScriptValue::vec3_type dir, float maxDistance)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        oo::Ray ray{ { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };
        
        RaycastResult result;
        try
        {
            result = ps->Raycast(ray, maxDistance);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        return result.Intersect;
    }

    SCRIPT_API bool Physics_Raycast_Filtered(ScriptValue::vec3_type origin, ScriptValue::vec3_type dir, float maxDistance, uint layerMask)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        oo::Ray ray{ { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };

        RaycastResult result;
        try
        {
            result = ps->Raycast(ray, maxDistance, layerMask);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        return result.Intersect;
    }

    SCRIPT_API ScriptDatabase::IntPtr Physics_RaycastBasic_WithData(ScriptValue::vec3_type origin, ScriptValue::vec3_type dir)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        oo::Ray ray{ { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };

        RaycastResult result;
        try
        {
            result = ps->Raycast(ray);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        if (!result.Intersect)
            return ScriptDatabase::InvalidPtr;
        return mono_gchandle_new(CreateRaycastHit(result), false);
    }

    SCRIPT_API ScriptDatabase::IntPtr Physics_Raycast_WithData(ScriptValue::vec3_type origin, ScriptValue::vec3_type dir, float maxDistance)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        oo::Ray ray{ { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };

        RaycastResult result;
        try
        {
            result = ps->Raycast(ray, maxDistance);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        if (!result.Intersect)
            return ScriptDatabase::InvalidPtr;
        return mono_gchandle_new(CreateRaycastHit(result), false);
    }

    SCRIPT_API ScriptDatabase::IntPtr Physics_Raycast_WithData_Filtered(ScriptValue::vec3_type origin, ScriptValue::vec3_type dir, float maxDistance, uint layerMask)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        oo::Ray ray{ { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };

        RaycastResult result;
        try
        {
            result = ps->Raycast(ray, maxDistance, layerMask);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        if (!result.Intersect)
            return ScriptDatabase::InvalidPtr;
        return mono_gchandle_new(CreateRaycastHit(result), false);
    }

    SCRIPT_API MonoArray* Physics_RaycastAllBasic(ScriptValue::vec3_type origin, ScriptValue::vec3_type dir)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        oo::Ray ray{ { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };

        std::vector<RaycastResult> result;
        try
        {
            result = ps->RaycastAll(ray);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        return CreateRaycastHitArray(result);
    }

    SCRIPT_API MonoArray* Physics_RaycastAll(ScriptValue::vec3_type origin, ScriptValue::vec3_type dir, float maxDistance)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        oo::Ray ray{ { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };

        std::vector<RaycastResult> result;
        try
        {
            result = ps->RaycastAll(ray, maxDistance);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        return CreateRaycastHitArray(result);
    }

    SCRIPT_API MonoArray* Physics_RaycastAll_Filtered(ScriptValue::vec3_type origin, ScriptValue::vec3_type dir, float maxDistance, uint layerMask)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        oo::Ray ray{ { origin.x, origin.y, origin.z }, { dir.x, dir.y, dir.z } };

        std::vector<RaycastResult> result;
        try
        {
            result = ps->RaycastAll(ray, maxDistance, layerMask);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        return CreateRaycastHitArray(result);
    }

    // For Rigidbody
    SCRIPT_API ScriptDatabase::IntPtr Rigidbody_SweepTest_Basic(Scene::ID_type sceneID, UUID uuid, ScriptValue::vec3_type dir)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();

        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        RigidbodyComponent& component = obj->GetComponent<RigidbodyComponent>();
        UUID physicsUUID = component.GetUnderlyingUUID();
        vec3 sweepDir{ dir.x, dir.y, dir.z };

        RaycastResult result;
        try
        {
            result = ps->Sweepcast(static_cast<phy_uuid::UUID>(physicsUUID), sweepDir);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        if (!result.Intersect)
            return ScriptDatabase::InvalidPtr;
        return mono_gchandle_new(CreateRaycastHit(result), false);
    }

    SCRIPT_API ScriptDatabase::IntPtr Rigidbody_SweepTest(Scene::ID_type sceneID, UUID uuid, ScriptValue::vec3_type dir, float maxDistance)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();

        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        RigidbodyComponent& component = obj->GetComponent<RigidbodyComponent>();
        UUID physicsUUID = component.GetUnderlyingUUID();
        vec3 sweepDir{ dir.x, dir.y, dir.z };

        RaycastResult result;
        try
        {
            result = ps->Sweepcast(static_cast<phy_uuid::UUID>(physicsUUID), sweepDir, maxDistance);
        }
        catch (std::exception const&)
        {
            // most likely invalid map key, do nothing
        }
        if (!result.Intersect)
            return ScriptDatabase::InvalidPtr;
        return mono_gchandle_new(CreateRaycastHit(result), false);
    }
}