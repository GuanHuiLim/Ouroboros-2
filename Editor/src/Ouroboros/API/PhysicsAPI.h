#pragma once

#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Scripting/ScriptManager.h"
#include "Ouroboros/Scripting/ScriptValue.h"

#include "Ouroboros/Physics/PhysicsSystem.h"

namespace oo
{
    // Helper Function to create a C# RaycastHit from a C++ RaycastResult
    ScriptDatabase::IntPtr CreateRaycastHit(RaycastResult& result)
    {
        ScriptSystem* ss = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<ScriptSystem>();

        MonoClass* dataClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "RaycastHit");
        MonoObject* hitInfo = ScriptEngine::CreateObject(dataClass);
        ScriptDatabase::IntPtr hitInfoPtr = mono_gchandle_new(hitInfo, false);

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

        return hitInfoPtr;
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
        return CreateRaycastHit(result);
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
        return CreateRaycastHit(result);
    }
}