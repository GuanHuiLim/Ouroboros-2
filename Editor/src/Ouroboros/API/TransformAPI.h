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
}