/************************************************************************************//*!
\file           ScriptSystem.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Declares the system responsible for handling the scene specific
                functionality of the scripting feature that varies between scenes

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <string>
#include <vector>
#include <filesystem>

#include <Scripting/Scripting.h>
#include "ScriptComponent.h"

#include "Ouroboros/ECS/GameObject.h"

namespace oo
{
    // Scene specific script stuff, go to ScriptManager for global stuff that transfers between scenes
    class ScriptSystem final
    {
    public:
        ScriptSystem(Scene& scene, ScriptDatabase& scripts, ComponentDatabase& components);
        ~ScriptSystem();

        bool StartPlay();
        void SetUpObject(UUID uuid);
        bool StopPlay();
        bool IsPlaying();

        void OnObjectEnabled(GameObjectComponent::OnEnableEvent* e);
        void OnObjectDisabled(GameObjectComponent::OnDisableEvent* e);
        void OnObjectDestroyed(GameObject::OnDestroy* e);

        void ResetScriptInfo(UUID uuid, ScriptComponent& script, ScriptClassInfo const& classInfo);
        void RefreshScriptInfoAll();

        ScriptDatabase::IntPtr AddScript(ScriptDatabase::UUID uuid, const char* name_space, const char* name);
        ScriptDatabase::IntPtr GetScript(ScriptDatabase::UUID uuid, const char* name_space, const char* name);
        void RemoveScript(ScriptDatabase::UUID uuid, const char* name_space, const char* name);
        void SetScriptEnabled(ScriptDatabase::UUID uuid, const char* name_space, const char* name, bool isEnabled);
        bool CheckScriptEnabled(ScriptDatabase::UUID uuid, const char* name_space, const char* name);

        ComponentDatabase::IntPtr AddComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name);
        ComponentDatabase::IntPtr GetComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name);
        void RemoveComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name);
        void SetComponentEnabled(ComponentDatabase::UUID uuid, const char* name_space, const char* name, bool isEnabled);
        bool CheckComponentEnabled(ComponentDatabase::UUID uuid, const char* name_space, const char* name);

        ComponentDatabase::IntPtr GetGameObject(ComponentDatabase::UUID uuid);

        void InvokeForObject(UUID uuid, const char* functionName, int paramCount = 0, void** params = NULL);
        void InvokeForObjectEnabled(UUID uuid, const char* functionName, int paramCount = 0, void** params = NULL);

        void InvokeForEach(const char* name_space, const char* name, const char* functionName, int paramCount = 0, void** params = NULL);
        void InvokeForEachEnabled(const char* name_space, const char* name, const char* functionName, int paramCount = 0, void** params = NULL);

        void InvokeForAll(const char* functionName, int paramCount = 0, void** params = NULL);
        void InvokeForAllEnabled(const char* functionName, int paramCount = 0, void** params = NULL);

    private:
        void SetUpObject(UUID uuid, ScriptComponent const& script);

        void UpdateAllScriptFieldsWithInfo();
        void UpdateScriptFieldsWithInfo(UUID uuid, ScriptComponent& script);

        Scene& scene;
        ScriptDatabase& scriptDatabase;
        ComponentDatabase& componentDatabase;
        bool isPlaying;
    };
}