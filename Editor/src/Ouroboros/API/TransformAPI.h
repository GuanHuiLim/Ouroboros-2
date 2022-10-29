/************************************************************************************//*!
\file           TransformAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to interact with the C++ Transform3D ECS System/Component

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"

#include "Ouroboros/Scripting/ScriptSystem.h"
#include "Ouroboros/Transform/TransformComponent.h"
#include "Ouroboros/Transform/TransformSystem.h"

namespace oo
{
    SCRIPT_API void Transform3D_GetLocalPosition(Scene::ID_type sceneID, oo::UUID uuid, float* x, float* y, float* z)
    { 
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        TransformComponent::vec3 vec3 = component.GetPosition();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_SetLocalPosition(Scene::ID_type sceneID, oo::UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        component.SetPosition({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->UpdateSubTree(*obj);
    }

    SCRIPT_API void Transform3D_GetGlobalPosition(Scene::ID_type sceneID, oo::UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        TransformComponent::vec3 vec3 = component.GetGlobalPosition();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_SetGlobalPosition(Scene::ID_type sceneID, oo::UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        component.SetGlobalPosition({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->UpdateSubTree(*obj);
    }

    SCRIPT_API void Transform3D_GetGlobalForward(Scene::ID_type sceneID, oo::UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        TransformComponent::vec3 vec3 = component.GlobalForward();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_GetGlobalLeft(Scene::ID_type sceneID, oo::UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        TransformComponent::vec3 vec3 = component.GlobalLeft();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_GetGlobalUp(Scene::ID_type sceneID, oo::UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        TransformComponent::vec3 vec3 = component.GlobalUp();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_GetLocalEulerAngles(Scene::ID_type sceneID, oo::UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        TransformComponent::vec3 vec3 = component.GetEulerAngles();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_LookAt(Scene::ID_type sceneID, oo::UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        component.LookAt({ x, y, z });
    }

    SCRIPT_API void Transform3D_SetLocalEulerAngles(Scene::ID_type sceneID, oo::UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        component.SetRotation({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->UpdateSubTree(*obj);
    }

    SCRIPT_API void Transform3D_GetGlobalEulerAngles(Scene::ID_type sceneID, oo::UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        TransformComponent::vec3 vec3 = component.GetGlobalRotationDeg();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_SetGlobalEulerAngles(Scene::ID_type sceneID, oo::UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        component.SetGlobalRotation({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->UpdateSubTree(*obj);
    }

    SCRIPT_API void Transform3D_GetLocalScale(Scene::ID_type sceneID, oo::UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        TransformComponent::vec3 vec3 = component.GetScale();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_SetLocalScale(Scene::ID_type sceneID, oo::UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        component.SetScale({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->UpdateSubTree(*obj);
    }

    SCRIPT_API void Transform3D_GetGlobalScale(Scene::ID_type sceneID, oo::UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        TransformComponent::vec3 vec3 = component.GetGlobalScale();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_SetGlobalScale(Scene::ID_type sceneID, oo::UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        TransformComponent& component = obj->GetComponent<TransformComponent>();
        component.SetGlobalScale({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->UpdateSubTree(*obj);
    }

    SCRIPT_API int Transform_GetChildCount(Scene::ID_type sceneID, oo::UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        return static_cast<int>(obj->GetDirectChildCount());
    }

    SCRIPT_API ComponentDatabase::IntPtr Transform_GetChild(Scene::ID_type sceneID, oo::UUID uuid, size_t childIndex)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        std::vector<oo::UUID> children = obj->GetDirectChildsUUID();
        if (childIndex >= children.size())
        {
            // Throw out of range exception?
            ScriptEngine::ThrowOutOfIndexException();
        }
        
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        return scene->GetWorld().Get_System<ScriptSystem>()->GetComponent(children[childIndex], "Ouroboros", "Transform");
    }

    SCRIPT_API ComponentDatabase::IntPtr Transform_FindChild(Scene::ID_type sceneID, oo::UUID uuid, const char* childPath)
    {
        GameObject obj = *(ScriptManager::GetObjectFromScene(sceneID, uuid));

        std::string pathString{ childPath };
        size_t curr = 0;
        size_t separator = pathString.find_first_of('/', curr);
        while (curr != std::string::npos)
        {
            std::string childName = pathString.substr(curr, separator - curr);
            bool childFound = false;
            for(GameObject child : obj.GetChildren())
            {
                if (child.Name() != childName)
                    continue;
                obj = child;
                childFound = true;
                break;
            }
            if (!childFound)
                return ComponentDatabase::InvalidPtr;

            if (separator == std::string::npos)
                break;
            curr = separator + 1;
            separator = pathString.find_first_of('/', curr);
        }

        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        return scene->GetWorld().Get_System<ScriptSystem>()->GetComponent(obj.GetInstanceID(), "Ouroboros", "Transform");
    }

    SCRIPT_API void Transform_SetParent(Scene::ID_type sceneID, oo::UUID uuid, oo::UUID newParent, bool preserveTransforms)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        if (newParent == GameObject::ROOTID)
        {
            scene->GetRoot()->AddChild(*obj, preserveTransforms);
            return;
        }
        std::shared_ptr<GameObject> parentObj = ScriptManager::GetObjectFromScene(sceneID, newParent);
        parentObj->AddChild(*obj, preserveTransforms);

        //manually update all transforms if set parent is called
        scene->GetWorld().Get_System<TransformSystem>()->UpdateSubTree(*obj);
    }

    SCRIPT_API uint32_t Transform_GetParent(Scene::ID_type sceneID, oo::UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        oo::UUID parentUUID = obj->GetParentUUID();

        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (parentUUID == GameObject::ROOTID)
            return ComponentDatabase::InvalidPtr;
        return scene->GetWorld().Get_System<ScriptSystem>()->GetComponent(parentUUID, "Ouroboros", "Transform");
    }
}