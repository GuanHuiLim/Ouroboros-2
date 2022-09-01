#pragma once
#include <Scripting/ExportAPI.h>

#include "Ouroboros/Scripting/ScriptSystem.h"
#include "Ouroboros/Transform/TransformComponent.h"
#include "Ouroboros/Transform/TransformSystem.h"

namespace oo
{
    SCRIPT_API void Transform3D_GetLocalPosition(Scene::ID_type sceneID, UUID uuid, float* x, float* y, float* z) 
    { 
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        Transform3D& component = obj->GetComponent<Transform3D>();
        Transform3D::vec3 vec3 = component.GetPosition();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_SetLocalPosition(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        Transform3D& component = obj->GetComponent<Transform3D>();
        component.SetPosition({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptSystem::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->Run(&(scene->GetWorld()));
    }

    SCRIPT_API void Transform3D_GetGlobalPosition(Scene::ID_type sceneID, UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        Transform3D& component = obj->GetComponent<Transform3D>();
        Transform3D::vec3 vec3 = component.GetGlobalPosition();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_SetGlobalPosition(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        Transform3D& component = obj->GetComponent<Transform3D>();
        component.SetGlobalPosition({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptSystem::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->Run(&(scene->GetWorld()));
    }

    SCRIPT_API void Transform3D_GetLocalEulerAngles(Scene::ID_type sceneID, UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        Transform3D& component = obj->GetComponent<Transform3D>();
        Transform3D::vec3 vec3 = component.GetGlobalPosition();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_SetLocalEulerAngles(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        Transform3D& component = obj->GetComponent<Transform3D>();
        component.SetGlobalPosition({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptSystem::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->Run(&(scene->GetWorld()));
    }

    SCRIPT_API void Transform3D_GetLocalScale(Scene::ID_type sceneID, UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        Transform3D& component = obj->GetComponent<Transform3D>();
        Transform3D::vec3 vec3 = component.GetScale();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_SetLocalScale(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        Transform3D& component = obj->GetComponent<Transform3D>();
        component.SetScale({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptSystem::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->Run(&(scene->GetWorld()));
    }

    SCRIPT_API void Transform3D_GetGlobalScale(Scene::ID_type sceneID, UUID uuid, float* x, float* y, float* z)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        Transform3D& component = obj->GetComponent<Transform3D>();
        Transform3D::vec3 vec3 = component.GetGlobalScale();
        *x = vec3.x;
        *y = vec3.y;
        *z = vec3.z;
    }

    SCRIPT_API void Transform3D_SetGlobalScale(Scene::ID_type sceneID, UUID uuid, float x, float y, float z)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        Transform3D& component = obj->GetComponent<Transform3D>();
        component.SetGlobalScale({ x, y, z });

        std::shared_ptr<Scene> scene = ScriptSystem::GetScene(sceneID);
        scene->GetWorld().Get_System<TransformSystem>()->Run(&(scene->GetWorld()));
    }

    SCRIPT_API int Transform_GetChildCount(Scene::ID_type sceneID, UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        return obj->GetDirectChildCount();
    }

    SCRIPT_API ComponentDatabase::IntPtr Transform_GetChild(Scene::ID_type sceneID, UUID uuid, size_t childIndex)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        std::vector<UUID> children = obj->GetDirectChildsUUID();
        if (childIndex >= children.size())
        {
            // Throw out of range exception?
            ScriptEngine::ThrowOutOfIndexException();
            // return 0;
        }
        
        std::shared_ptr<Scene> scene = ScriptSystem::GetScene(sceneID);
        return scene->GetWorld().Get_System<ScriptSystem>()->GetComponent(children[childIndex], "Ouroboros", "Transform");
    }

    SCRIPT_API void Transform_SetParent(Scene::ID_type sceneID, UUID uuid, UUID newParent, bool preserveTransforms)
    {
        std::shared_ptr<Scene> scene = ScriptSystem::GetScene(sceneID);
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        if (newParent == GameObject::ROOTID)
        {
            scene->GetRoot()->AddChild(*obj, preserveTransforms);
            return;
        }
        std::shared_ptr<GameObject> parentObj = ScriptSystem::GetObjectFromScene(sceneID, newParent);
        parentObj->AddChild(*obj, preserveTransforms);

        //manually update all transforms if set parent is called
        scene->GetWorld().Get_System<TransformSystem>()->Run(&(scene->GetWorld()));
    }

    SCRIPT_API uint32_t Transform_GetParent(Scene::ID_type sceneID, UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        UUID parentUUID = obj->GetParentUUID();

        std::shared_ptr<Scene> scene = ScriptSystem::GetScene(sceneID);
        if (parentUUID == GameObject::ROOTID)
            return 0;
        return scene->GetWorld().Get_System<ScriptSystem>()->GetComponent(parentUUID, "Ouroboros", "Transform");
    }
}