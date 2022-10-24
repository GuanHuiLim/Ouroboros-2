/************************************************************************************//*!
\file           ScriptSystem.cpp
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Defines the system responsible for handling the scene specific
                functionality of the scripting feature that varies between scenes

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "ScriptSystem.h"

#include "ScriptManager.h"

#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/Scene/EditorController.h"

#include "Ouroboros/ECS/ECS.h"

namespace oo
{
    // Scene specific script stuff
    ScriptSystem::ScriptSystem(Scene& scene, ScriptDatabase& scripts, ComponentDatabase& components)
    : scene{ scene }, scriptDatabase{ scripts }, componentDatabase{ components }, isPlaying{ false }
    {
        EventManager::Subscribe<ScriptSystem, GameObjectComponent::OnEnableEvent>(this, &ScriptSystem::OnObjectEnabled);
        EventManager::Subscribe<ScriptSystem, GameObjectComponent::OnDisableEvent>(this, &ScriptSystem::OnObjectDisabled);
        EventManager::Subscribe<ScriptSystem, GameObject::OnDestroy>(this, &ScriptSystem::OnObjectDestroyed);

        EventManager::Subscribe<ScriptSystem, PhysicsTickEvent>(this, &ScriptSystem::OnPhysicsTick);
        EventManager::Subscribe<ScriptSystem, PhysicsTriggerEvent>(this, &ScriptSystem::OnTriggerEvent);
    }

    ScriptSystem::~ScriptSystem()
    {
        EventManager::Unsubscribe<ScriptSystem, GameObjectComponent::OnEnableEvent>(this, &ScriptSystem::OnObjectEnabled);
        EventManager::Unsubscribe<ScriptSystem, GameObjectComponent::OnDisableEvent>(this, &ScriptSystem::OnObjectDisabled);
        EventManager::Unsubscribe<ScriptSystem, GameObject::OnDestroy>(this, &ScriptSystem::OnObjectDestroyed);

        EventManager::Unsubscribe<ScriptSystem, PhysicsTickEvent>(this, &ScriptSystem::OnPhysicsTick);
        EventManager::Unsubscribe<ScriptSystem, PhysicsTriggerEvent>(this, &ScriptSystem::OnTriggerEvent);

        scriptDatabase.DeleteAll();
        componentDatabase.DeleteAll();
    }

    bool ScriptSystem::StartPlay()
    {
        if (isPlaying)
            return false;
        if (!ScriptEngine::IsLoaded())
            return false;
        if (ScriptManager::DisplayErrors())
            return false;

        std::vector<MonoClass*> executionOrder = ScriptManager::GetScriptExecutionOrder();
        if (executionOrder.size() <= 0)
        {
            LOG_WARN("ScriptSystem: No scripts found, ScriptSystem functions will not be run");
            return true;
        }
        componentDatabase.Initialize();
        scriptDatabase.Initialize(executionOrder);

        static Ecs::Query query = Ecs::make_raw_query<GameObjectComponent, ScriptComponent>();

        isPlaying = true;
        scene.GetWorld().for_each(query, [&](GameObjectComponent& gameObject, ScriptComponent& script)
            {
                if (gameObject.Id == GameObject::ROOTID)
                    return;
                SetUpObject(gameObject.Id, script);
            });
        UpdateAllScriptFieldsWithInfo();
        InvokeForAll("Awake");
        InvokeForAll("Start");
        return true;
    }

    void ScriptSystem::SetUpObject(oo::UUID uuid)
    {
        std::shared_ptr<GameObject> gameObject = scene.FindWithInstanceID(uuid);
        ASSERT(gameObject == nullptr);
        SetUpObject(uuid, gameObject->GetComponent<ScriptComponent>());
    }
    void ScriptSystem::SetUpObject(oo::UUID uuid, ScriptComponent const& script)
    {
        componentDatabase.InstantiateObjectFull(uuid);
        // Add all scripts
        for (auto const& [key, scriptInfo] : script.GetScriptInfoAll())
        {
            AddScript(uuid, scriptInfo.classInfo.name_space.c_str(), scriptInfo.classInfo.name.c_str());
        }
    }

    bool ScriptSystem::StopPlay()
    {
        if (!isPlaying)
            return false;

        isPlaying = false;
        scriptDatabase.DeleteAll();
        componentDatabase.DeleteAll();
        return true;
    }

    bool ScriptSystem::IsPlaying()
    {
        return isPlaying;
    }

    void ScriptSystem::ResetScriptInfo(UUID uuid, ScriptComponent& script, ScriptClassInfo const& classInfo)
    {
        auto& scriptInfoMap = script.GetScriptInfoAll();
        auto search = scriptInfoMap.find(classInfo.ToString());
        if (search == scriptInfoMap.end())
            return;
        search->second.ResetFieldValues();
        if (isPlaying)
            UpdateScriptFieldsWithInfo(uuid, script);
    }
    void ScriptSystem::RefreshScriptInfoAll()
    {
        static Ecs::Query query = Ecs::make_raw_query<GameObjectComponent, ScriptComponent>();

        scene.GetWorld().for_each(query, [&](GameObjectComponent& gameObject, ScriptComponent& script)
            {
                if (gameObject.Id == GameObject::ROOTID)
                    return;
                auto& scriptInfoMap = script.GetScriptInfoAll();

                std::vector<std::string> eraseKeys;
                for (auto& [key, scriptInfo] : scriptInfoMap)
                {
                    if (!ScriptEngine::CheckClassExists("Scripting", scriptInfo.classInfo.name_space.c_str(), scriptInfo.classInfo.name.c_str()))
                    {
                        eraseKeys.emplace_back(key);
                        continue;
                    }
                    ScriptInfo newInfo(scriptInfo.classInfo);
                    newInfo.CopyFieldValues(scriptInfo);
                    scriptInfo = newInfo;
                }

                if (eraseKeys.size() > 0)
                {
                    for (std::string const& key : eraseKeys)
                    {
                        auto search = scriptInfoMap.find(key);
                        if (search == scriptInfoMap.end())
                            continue;
                        LOG_TRACE("no script class found with name {0} in {1}, script was removed", search->second.classInfo.ToString(), gameObject.Name);
                        scriptInfoMap.erase(search);
                    }
                }
            });
    }

    ScriptDatabase::IntPtr ScriptSystem::AddScript(ScriptDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!isPlaying)
            return 0;

        // create script instance
        ScriptDatabase::IntPtr ptr = scriptDatabase.Instantiate(uuid, name_space, name);
        MonoObject* script = mono_gchandle_get_target(ptr);

        // set gameObject field
        MonoClass* klass = ScriptEngine::GetClass("Scripting", name_space, name);
        MonoClassField* gameObjectField = mono_class_get_field_from_name(klass, "m_GameObject");
        MonoObject* gameObject = componentDatabase.RetrieveGameObjectObject(uuid);
        mono_field_set_value(script, gameObjectField, gameObject);

        // set componentID field
        //std::string key = std::string{ name_space } + "." + name;
        //MonoClassField* idField = mono_class_get_field_from_name(klass, "m_ComponentID");
        //mono_field_set_value(script, idField, &key);

        return ptr;
    }
    ScriptDatabase::IntPtr ScriptSystem::GetScript(ScriptDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!isPlaying)
            return 0;
        return scriptDatabase.TryRetrieveDerived(uuid, name_space, name);
    }
    void ScriptSystem::RemoveScript(ScriptDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!isPlaying)
            return;
        scriptDatabase.Delete(uuid, name_space, name);
    }
    void ScriptSystem::SetScriptEnabled(ScriptDatabase::UUID uuid, const char* name_space, const char* name, bool isEnabled)
    {
        if (!isPlaying)
            return;
        scriptDatabase.SetEnabled(uuid, name_space, name, isEnabled);
        std::shared_ptr<GameObject> gameObject = scene.FindWithInstanceID(uuid);
        if (!gameObject->ActiveInHierarchy())
            return;
        MonoObject* obj = scriptDatabase.RetrieveObject(uuid, name_space, name);
        if(isEnabled)
            ScriptEngine::InvokeFunction(obj, "OnEnable");
        else
            ScriptEngine::InvokeFunction(obj, "OnDisable");
    }
    bool ScriptSystem::CheckScriptEnabled(ScriptDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!isPlaying)
            return false;
        return scriptDatabase.CheckEnabled(uuid, name_space, name);
    }

    ComponentDatabase::IntPtr ScriptSystem::AddComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!isPlaying)
            return 0;
        return componentDatabase.Instantiate(uuid, name_space, name);
    }
    ComponentDatabase::IntPtr ScriptSystem::GetComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!isPlaying)
            return 0;
        return componentDatabase.TryRetrieveDerived(uuid, name_space, name);
    }
    void ScriptSystem::RemoveComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!isPlaying)
            return;
        componentDatabase.Delete(uuid, name_space, name);
    }
    void ScriptSystem::SetComponentEnabled(ComponentDatabase::UUID uuid, const char* name_space, const char* name, bool isEnabled)
    {
        if (!isPlaying)
            return;
        componentDatabase.SetEnabled(uuid, name_space, name, isEnabled);
    }
    bool ScriptSystem::CheckComponentEnabled(ComponentDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!isPlaying)
            return false;
        return componentDatabase.CheckEnabled(uuid, name_space, name);
    }

    ComponentDatabase::IntPtr ScriptSystem::GetGameObject(ComponentDatabase::UUID uuid)
    {
        if (!isPlaying)
            return 0;
        return componentDatabase.RetrieveGameObject(uuid);
    }

    void ScriptSystem::InvokeForObject(oo::UUID uuid, const char* functionName, int paramCount, void** params)
    {
        if (!ScriptEngine::IsLoaded())
            return;
        if (!isPlaying)
            return;
        scriptDatabase.ForEach(uuid, [&functionName, &params, &paramCount](MonoObject* object)
            {
                try
                {
                    ScriptEngine::InvokeFunction(object, functionName, params, paramCount);
                }
                catch (std::exception const& e)
                {
                    LOG_ERROR(e.what());
                }
            });
    }
    void ScriptSystem::InvokeForObjectEnabled(oo::UUID uuid, const char* functionName, int paramCount, void** params)
    {
        if (!ScriptEngine::IsLoaded())
            return;
        if (!isPlaying)
            return;
        scriptDatabase.ForEachEnabled(uuid, [&functionName, &params, &paramCount](MonoObject* object)
            {
                try
                {
                    ScriptEngine::InvokeFunction(object, functionName, params, paramCount);
                }
                catch (std::exception const& e)
                {
                    LOG_ERROR(e.what());
                }
            }, [this](ScriptDatabase::UUID uuid)
            {
                std::shared_ptr<GameObject> object = scene.FindWithInstanceID(uuid);
                return object->ActiveInHierarchy();
            });
    }

    void ScriptSystem::InvokeForEach(const char* name_space, const char* name, const char* functionName, int paramCount, void** params)
    {
        if (!ScriptEngine::IsLoaded())
            return;
        if (!isPlaying)
            return;
        scriptDatabase.ForEach(name_space, name, [&functionName, &params, &paramCount](MonoObject* object)
            {
                try
                {
                    ScriptEngine::InvokeFunction(object, functionName, params, paramCount);
                }
                catch (std::exception const& e)
                {
                    LOG_ERROR(e.what());
                }
            });
    }
    void ScriptSystem::InvokeForEachEnabled(const char* name_space, const char* name, const char* functionName, int paramCount, void** params)
    {
        if (!ScriptEngine::IsLoaded())
            return;
        if (!isPlaying)
            return;
        scriptDatabase.ForEach(name_space, name, [&functionName, &params, &paramCount](MonoObject* object)
            {
                try
                {
                    ScriptEngine::InvokeFunction(object, functionName, params, paramCount);
                }
                catch (std::exception const& e)
                {
                    LOG_ERROR(e.what());
                }
            }, [this](ScriptDatabase::UUID uuid)
            {
                std::shared_ptr<GameObject> object = scene.FindWithInstanceID(uuid);
                return object->ActiveInHierarchy();
            });
    }

    void ScriptSystem::InvokeForAll(const char* functionName, int paramCount, void** params)
    {
        if (!ScriptEngine::IsLoaded())
            return;
        if (!isPlaying)
            return;
        scriptDatabase.ForAll([&functionName, &params, &paramCount](MonoObject* object)
            {
                try
                {
                    ScriptEngine::InvokeFunction(object, functionName, params, paramCount);
                }
                catch (std::exception const& e)
                {
                    LOG_ERROR(e.what());
                }
            });
    }
    void ScriptSystem::InvokeForAllEnabled(const char* functionName, int paramCount, void** params)
    {
        if (!ScriptEngine::IsLoaded())
            return;
        if (!isPlaying)
            return;
        scriptDatabase.ForAllEnabled([&functionName, &params, &paramCount](MonoObject* object)
            {
                try
                {
                    ScriptEngine::InvokeFunction(object, functionName, params, paramCount);
                }
                catch (std::exception const& e)
                {
                    LOG_ERROR(e.what());
                }
            }, [this](ScriptDatabase::UUID uuid)
            {
                std::shared_ptr<GameObject> object = scene.FindWithInstanceID(uuid);
                return object->ActiveInHierarchy();
            });
    }

    void ScriptSystem::UpdateAllScriptFieldsWithInfo()
    {
        static Ecs::Query query = Ecs::make_raw_query<GameObjectComponent, ScriptComponent>();
        scene.GetWorld().for_each(query, [&](GameObjectComponent& gameObject, ScriptComponent& script)
            {
                if (gameObject.Id == GameObject::ROOTID)
                    return;
                UpdateScriptFieldsWithInfo(gameObject.Id, script);
            });
    }
    void ScriptSystem::UpdateScriptFieldsWithInfo(oo::UUID uuid, ScriptComponent& script)
    {
        for (auto& [scriptKey, scriptInfo] : script.GetScriptInfoAll())
        {
            MonoObject* scriptObj = scriptDatabase.RetrieveObject(uuid, scriptInfo.classInfo.name_space.c_str(), scriptInfo.classInfo.name.c_str());
            MonoClass* scriptClass = ScriptEngine::GetClass("Scripting", scriptInfo.classInfo.name_space.c_str(), scriptInfo.classInfo.name.c_str());
            for (auto& [fieldKey, fieldInfo] : scriptInfo.fieldMap)
            {
                MonoClassField* field = mono_class_get_field_from_name(scriptClass, fieldInfo.name.c_str());
                ScriptValue::SetFieldValue(scriptObj, field, fieldInfo.value);
                fieldInfo.SetScriptReference(field, scriptObj);
            }
        }
    }

    void ScriptSystem::OnObjectEnabled(GameObjectComponent::OnEnableEvent* e)
    {
        if (scene.FindWithInstanceID(e->Id) == nullptr)
            return;
        InvokeForObject(e->Id, "OnEnable");
    }
    void ScriptSystem::OnObjectDisabled(GameObjectComponent::OnDisableEvent* e)
    {
        if (scene.FindWithInstanceID(e->Id) == nullptr)
            return;
        InvokeForObject(e->Id, "OnDisable");
    }
    void ScriptSystem::OnObjectDestroyed(GameObject::OnDestroy* e)
    {
        UUID uuid = e->go->GetInstanceID();
        if (scene.FindWithInstanceID(uuid) == nullptr)
            return;
        scriptDatabase.Delete(uuid);
        componentDatabase.Delete(uuid);
    }

    void ScriptSystem::OnPhysicsTick(PhysicsTickEvent* e)
    {
        InvokeForAllEnabled("FixedUpdate");
    }

    void ScriptSystem::OnTriggerEvent(PhysicsTriggerEvent* e)
    {
        MonoObject* obj = componentDatabase.TryRetrieveDerivedObject(e->TriggerID, "Ouroboros", "Collider");
        MonoObject* other = componentDatabase.TryRetrieveDerivedObject(e->OtherID, "Ouroboros", "Collider");

        void* objParams[1];
        objParams[0] = other;
        void* otherParams[1];
        otherParams[0] = obj;

        MonoMethod* method = nullptr;
        switch (e->State)
        {
        case TriggerState::ENTER:
        {
            scriptDatabase.ForEachEnabled(e->TriggerID, [&objParams](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnTriggerEnter", objParams, 1);
                });
            scriptDatabase.ForEachEnabled(e->OtherID, [&otherParams](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnTriggerEnter", otherParams, 1);
                });
        }
        break;
        case TriggerState::STAY:
        {
            scriptDatabase.ForEachEnabled(e->TriggerID, [&objParams](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnTriggerStay", objParams, 1);
                });
            scriptDatabase.ForEachEnabled(e->OtherID, [&otherParams](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnTriggerStay", otherParams, 1);
                });
        }
        break;
        case TriggerState::EXIT:
        {
            scriptDatabase.ForEachEnabled(e->TriggerID, [&objParams](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnTriggerExit", objParams, 1);
                });
            scriptDatabase.ForEachEnabled(e->OtherID, [&otherParams](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnTriggerExit", otherParams, 1);
                });
        }
        break;
        }
    }
}