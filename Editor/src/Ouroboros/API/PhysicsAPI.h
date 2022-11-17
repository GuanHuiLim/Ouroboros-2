#pragma once

#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Scripting/ScriptManager.h"
#include "Ouroboros/Scripting/ScriptValue.h"

#include "Ouroboros/Physics/PhysicsSystem.h"

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
            glm::vec3* vecSetter = reinterpret_cast<glm::vec3*>(element);
            *vecSetter = result.Position;

            element += sizeof(float) * 3;
            vecSetter = reinterpret_cast<glm::vec3*>(element);
            *vecSetter = result.Normal;

            element += sizeof(float) * 3;
            float* floatSetter = reinterpret_cast<float*>(element);
            *floatSetter = result.Distance;
;        }
        return arr;
    }

    SCRIPT_API bool Physics_RaycastBasic(glm::vec3 origin, glm::vec3 dir)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        RaycastResult result = ps->Raycast(oo::Ray{ origin, dir });
        return result.Intersect;
    }

    SCRIPT_API bool Physics_Raycast(glm::vec3 origin, glm::vec3 dir, float maxDistance)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();
        RaycastResult result = ps->Raycast(oo::Ray{ origin, dir }, maxDistance);
        return result.Intersect;
    }

    SCRIPT_API ScriptDatabase::IntPtr Physics_RaycastBasic_WithData(glm::vec3 origin, glm::vec3 dir)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();

        RaycastResult result;
        try
        {
            result = ps->Raycast(oo::Ray{ origin, dir });
        }
        catch (std::exception const& e)
        {
            // most likely invalid map key, do nothing
        }
        if (!result.Intersect)
            return ScriptDatabase::InvalidPtr;
        return mono_gchandle_new(CreateRaycastHit(result), false);
    }

    SCRIPT_API ScriptDatabase::IntPtr Physics_Raycast_WithData(glm::vec3 origin, glm::vec3 dir, float maxDistance)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();

        RaycastResult result;
        try
        {
            result = ps->Raycast(oo::Ray{ origin, dir }, maxDistance);
        }
        catch (std::exception const& e)
        {
            // most likely invalid map key, do nothing
        }
        if (!result.Intersect)
            return ScriptDatabase::InvalidPtr;
        return mono_gchandle_new(CreateRaycastHit(result), false);
    }

    SCRIPT_API MonoArray* Physics_RaycastAllBasic(glm::vec3 origin, glm::vec3 dir)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();

        std::vector<RaycastResult> result;
        try
        {
            result = ps->RaycastAll(oo::Ray{ origin, dir });
        }
        catch (std::exception const& e)
        {
            // most likely invalid map key, do nothing
        }
        return CreateRaycastHitArray(result);
    }

    SCRIPT_API MonoArray* Physics_RaycastAll(glm::vec3 origin, glm::vec3 dir, float maxDistance)
    {
        PhysicsSystem* ps = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<PhysicsSystem>();

        std::vector<RaycastResult> result;
        try
        {
            result = ps->RaycastAll(oo::Ray{ origin, dir }, maxDistance);
        }
        catch (std::exception const& e)
        {
            // most likely invalid map key, do nothing
        }
        return CreateRaycastHitArray(result);
    }
}