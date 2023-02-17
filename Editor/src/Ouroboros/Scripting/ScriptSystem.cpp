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

#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

namespace oo
{
    // Scene specific script stuff
    ScriptSystem::ScriptSystem(Scene& scene, ScriptDatabase& scripts, ComponentDatabase& components)
    : scene{ scene }, scriptDatabase{ scripts }, componentDatabase{ components }, isPlaying{ false }
    {
        EventManager::Subscribe<ScriptSystem, WindowFocusEvent>(this, &ScriptSystem::OnApplicationFocus);
        EventManager::Subscribe<ScriptSystem, WindowLoseFocusEvent>(this, &ScriptSystem::OnApplicationPause);

        EventManager::Subscribe<ScriptSystem, GameObjectComponent::OnEnableEvent>(this, &ScriptSystem::OnObjectEnabled);
        EventManager::Subscribe<ScriptSystem, GameObjectComponent::OnDisableEvent>(this, &ScriptSystem::OnObjectDisabled);
        EventManager::Subscribe<ScriptSystem, GameObject::OnDestroy>(this, &ScriptSystem::OnObjectDestroyed);

        EventManager::Subscribe<ScriptSystem, PhysicsTickEvent>(this, &ScriptSystem::OnPhysicsTick);
        //EventManager::Subscribe<ScriptSystem, PhysicsTriggerEvent>(this, &ScriptSystem::OnTriggerEvent);
        //EventManager::Subscribe<ScriptSystem, PhysicsCollisionEvent>(this, &ScriptSystem::OnCollisionEvent);

        EventManager::Subscribe<ScriptSystem, PhysicsTriggersEvent>(this, &ScriptSystem::OnTriggerAllEvent);
        EventManager::Subscribe<ScriptSystem, PhysicsCollisionsEvent>(this, &ScriptSystem::OnCollisionAllEvent);
    }

    ScriptSystem::~ScriptSystem()
    {
        EventManager::Unsubscribe<ScriptSystem, WindowFocusEvent>(this, &ScriptSystem::OnApplicationFocus);
        EventManager::Unsubscribe<ScriptSystem, WindowLoseFocusEvent>(this, &ScriptSystem::OnApplicationPause);

        EventManager::Unsubscribe<ScriptSystem, GameObjectComponent::OnEnableEvent>(this, &ScriptSystem::OnObjectEnabled);
        EventManager::Unsubscribe<ScriptSystem, GameObjectComponent::OnDisableEvent>(this, &ScriptSystem::OnObjectDisabled);
        EventManager::Unsubscribe<ScriptSystem, GameObject::OnDestroy>(this, &ScriptSystem::OnObjectDestroyed);

        EventManager::Unsubscribe<ScriptSystem, PhysicsTickEvent>(this, &ScriptSystem::OnPhysicsTick);
        //EventManager::Unsubscribe<ScriptSystem, PhysicsTriggerEvent>(this, &ScriptSystem::OnTriggerEvent);
        //EventManager::Unsubscribe<ScriptSystem, PhysicsCollisionEvent>(this, &ScriptSystem::OnCollisionEvent);

        EventManager::Unsubscribe<ScriptSystem, PhysicsTriggersEvent>(this, &ScriptSystem::OnTriggerAllEvent);
        EventManager::Unsubscribe<ScriptSystem, PhysicsCollisionsEvent>(this, &ScriptSystem::OnCollisionAllEvent);

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
    void ScriptSystem::UpdateObjectFieldsWithInfo(oo::UUID uuid)
    {
        std::shared_ptr<GameObject> gameObject = scene.FindWithInstanceID(uuid);
        ASSERT(gameObject == nullptr);
        UpdateScriptFieldsWithInfo(uuid, gameObject->GetComponent<ScriptComponent>());
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

    void ScriptSystem::ResetScriptInfo(oo::UUID uuid, ScriptComponent& script, ScriptClassInfo const& classInfo)
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
        MonoObject* script = scriptDatabase.TryRetrieveObject(uuid, name_space, name);
        if (script == nullptr)
            return;
        ScriptEngine::InvokeFunction(script, "OnDestroy");
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
    ComponentDatabase::IntPtr ScriptSystem::HasActualComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name)
    {
        return componentDatabase.HasComponent(uuid, name_space, name);
    }
    void ScriptSystem::RemoveComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!isPlaying)
            return;
        componentDatabase.Delete(uuid, name_space, name);
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
    void ScriptSystem::InvokeForAllEnabled(const char* functionName)
    {
        if (!ScriptEngine::IsLoaded())
            return;
        if (!isPlaying)
            return;

        try
        {
            scriptDatabase.InvokeForAllEnabled(functionName,
                [this](ScriptDatabase::UUID uuid)
                {
                    std::shared_ptr<GameObject> object = scene.FindWithInstanceID(uuid);
                    return object->ActiveInHierarchy();
                });
        }
        catch (std::exception const& e)
        {
            LOG_ERROR(e.what());
        }
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
            ScriptDatabase::IntPtr scriptPtr = scriptDatabase.Retrieve(uuid, scriptInfo.classInfo.name_space.c_str(), scriptInfo.classInfo.name.c_str());
            MonoObject* scriptObj = mono_gchandle_get_target(scriptPtr);
            MonoClass* scriptClass = ScriptEngine::GetClass("Scripting", scriptInfo.classInfo.name_space.c_str(), scriptInfo.classInfo.name.c_str());
            for (auto& [fieldKey, fieldInfo] : scriptInfo.fieldMap)
            {
                MonoClassField* field = mono_class_get_field_from_name(scriptClass, fieldInfo.name.c_str());
                ScriptValue::SetFieldValue(scriptObj, field, fieldInfo.value);
                fieldInfo.SetScriptReference(field, scriptPtr);
            }
        }
    }

    void ScriptSystem::OnApplicationFocus(WindowFocusEvent* e)
    {
        if (!isPlaying)
            return;
        InvokeForAllEnabled("OnApplicationFocus");
    }
    void ScriptSystem::OnApplicationPause(WindowLoseFocusEvent* e)
    {
        if (!isPlaying)
            return;
        InvokeForAllEnabled("OnApplicationPause");
    }

    void ScriptSystem::OnObjectEnabled(GameObjectComponent::OnEnableEvent* e)
    {
        if (!isPlaying)
            return;
        if (scene.FindWithInstanceID(e->Id) == nullptr)
            return;
        InvokeForObject(e->Id, "OnEnable");
    }
    void ScriptSystem::OnObjectDisabled(GameObjectComponent::OnDisableEvent* e)
    {
        if (!isPlaying)
            return;
        if (scene.FindWithInstanceID(e->Id) == nullptr)
            return;
        InvokeForObject(e->Id, "OnDisable");
    }
    void ScriptSystem::OnObjectDestroyed(GameObject::OnDestroy* e)
    {
        if (!isPlaying)
            return;
        UUID uuid = e->go->GetInstanceID();
        if (scene.FindWithInstanceID(uuid) == nullptr)
            return;
        scriptDatabase.ForEachEnabled(uuid, [](MonoObject* object)
            {
                ScriptEngine::InvokeFunction(object, "OnDestroy");
            });
        scriptDatabase.Delete(uuid);
        componentDatabase.Delete(uuid);
    }

    void ScriptSystem::OnPhysicsTick(PhysicsTickEvent* e)
    {
        if (!isPlaying)
            return;
        TRACY_PROFILE_SCOPE(scripts_fixed_update);
        InvokeForAllEnabled("FixedUpdate");
        TRACY_PROFILE_SCOPE_END();
    }

    void ScriptSystem::OnTriggerEvent(PhysicsTriggerEvent* e)
    {
        if (!isPlaying)
            return;
        MonoObject* obj = componentDatabase.TryRetrieveDerivedObject(e->TriggerID, "Ouroboros", "Collider");
        MonoObject* other = componentDatabase.TryRetrieveDerivedObject(e->OtherID, "Ouroboros", "Collider");

        if (obj == nullptr || other == nullptr)
        {
            LOG_CORE_ERROR("ScriptSystem Error: TriggerEvent Broadcasted, but Collider involved not supported in C#");
            return;
        }

        void* objParams[1];
        objParams[0] = other;
        void* otherParams[1];
        otherParams[0] = obj;

        switch (e->State)
        {
        case PhysicsEventState::ENTER:
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
        case PhysicsEventState::STAY:
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
        case PhysicsEventState::EXIT:
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

    void ScriptSystem::OnCollisionEvent(PhysicsCollisionEvent* e)
    {
        if (!isPlaying)
            return;

        struct TempContactPoint
        {
            ScriptValue::vec3_type normal;
            ScriptValue::vec3_type point;
            ScriptValue::vec3_type impulse;
        };

        MonoObject* col1 = componentDatabase.TryRetrieveDerivedObject(e->Collider1, "Ouroboros", "Collider");
        MonoObject* col2 = componentDatabase.TryRetrieveDerivedObject(e->Collider2, "Ouroboros", "Collider");

        if (col1 == nullptr || col2 == nullptr)
        {
            LOG_CORE_ERROR("ScriptSystem Error: CollisionEvent Broadcasted, but Collider involved not supported in C#");
            return;
        }

        MonoClass* dataClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Collision");
        MonoClassField* field = nullptr;

        MonoClass* contactPointClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "ContactPoint");
        MonoArray* arr = ScriptEngine::CreateArray(contactPointClass, e->ContactCount);
        TempContactPoint temp;
        for (size_t i = 0; i < e->ContactCount; ++i)
        {
            temp.normal = ScriptValue::vec3_type{ e->ContactPoints[i].Normal.x, e->ContactPoints[i].Normal.y, e->ContactPoints[i].Normal.z };
            temp.point = ScriptValue::vec3_type{ e->ContactPoints[i].Point.x, e->ContactPoints[i].Point.y, e->ContactPoints[i].Point.z };
            temp.impulse = ScriptValue::vec3_type{ e->ContactPoints[i].Impulse.x, e->ContactPoints[i].Impulse.y, e->ContactPoints[i].Impulse.z };
            mono_array_set(arr, TempContactPoint, i, temp);
        }

        // Collision Data for Collider1

        MonoObject* collisionData1 = ScriptEngine::CreateObject(dataClass);

        field = mono_class_get_field_from_name(dataClass, "m_gameObject");
        mono_field_set_value(collisionData1, field, componentDatabase.TryRetrieveGameObjectObject(e->Collider1));
        field = mono_class_get_field_from_name(dataClass, "m_rigidbody");
        mono_field_set_value(collisionData1, field, componentDatabase.TryRetrieveDerivedObject(e->Collider1, "Ouroboros", "Rigidbody"));
        field = mono_class_get_field_from_name(dataClass, "m_collider");
        mono_field_set_value(collisionData1, field, col1);

        field = mono_class_get_field_from_name(dataClass, "m_contacts");
        mono_field_set_value(collisionData1, field, arr);

        // Collision Data for Collider2

        MonoObject* collisionData2 = ScriptEngine::CreateObject(dataClass);

        field = mono_class_get_field_from_name(dataClass, "m_gameObject");
        mono_field_set_value(collisionData2, field, componentDatabase.TryRetrieveGameObjectObject(e->Collider2));
        field = mono_class_get_field_from_name(dataClass, "m_rigidbody");
        mono_field_set_value(collisionData2, field, componentDatabase.TryRetrieveDerivedObject(e->Collider2, "Ouroboros", "Rigidbody"));
        field = mono_class_get_field_from_name(dataClass, "m_collider");
        mono_field_set_value(collisionData2, field, col2);

        field = mono_class_get_field_from_name(dataClass, "m_contacts");
        mono_field_set_value(collisionData2, field, arr);

        void* params1[1];
        params1[0] = collisionData1;

        void* params2[1];
        params2[0] = collisionData2;

        switch (e->State)
        {
        case PhysicsEventState::ENTER:
        {
            scriptDatabase.ForEachEnabled(e->Collider1, [&params2](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnCollisionEnter", params2, 1);
                });
            scriptDatabase.ForEachEnabled(e->Collider2, [&params1](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnCollisionEnter", params1, 1);
                });
        }
        break;
        case PhysicsEventState::STAY:
        {
            scriptDatabase.ForEachEnabled(e->Collider1, [&params2](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnCollisionStay", params2, 1);
                });
            scriptDatabase.ForEachEnabled(e->Collider2, [&params1](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnCollisionStay", params1, 1);
                });
        }
        break;
        case PhysicsEventState::EXIT:
        {
            scriptDatabase.ForEachEnabled(e->Collider1, [&params2](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnCollisionExit", params2, 1);
                });
            scriptDatabase.ForEachEnabled(e->Collider2, [&params1](MonoObject* script)
                {
                    ScriptEngine::InvokeFunction(script, "OnCollisionExit", params1, 1);
                });
        }
        break;
        }
    }

    void ScriptSystem::OnTriggerAllEvent(PhysicsTriggersEvent* e)
    {
        for (PhysicsTriggerEvent& ev : e->TriggerEvents)
        {
            OnTriggerEvent(&ev);
        }
    }

    void ScriptSystem::OnCollisionAllEvent(PhysicsCollisionsEvent* e)
    {
        //for (PhysicsCollisionEvent& ev : e->CollisionEvents)
        //{
        //    OnCollisionEvent(&ev);
        //}

        struct TempContactPoint
        {
            ScriptValue::vec3_type normal;
            ScriptValue::vec3_type point;
            ScriptValue::vec3_type impulse;
        };
        struct CallbackData
        {
            MonoObject* data;
            PhysicsEventState state;
        };

        if (!isPlaying)
            return;

        MonoClass* contactPointClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "ContactPoint");
        MonoClass* dataClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Collision");
        MonoClassField* dataClassGameObjectField = mono_class_get_field_from_name(dataClass, "m_gameObject");
        MonoClassField* dataClassRigidbodyField = mono_class_get_field_from_name(dataClass, "m_rigidbody");
        MonoClassField* dataClassColliderField = mono_class_get_field_from_name(dataClass, "m_collider");
        MonoClassField* dataClassContactsField = mono_class_get_field_from_name(dataClass, "m_contacts");

        std::unordered_map<UUID, std::vector<CallbackData>> scriptDataMap;
        scriptDataMap.reserve(e->CollisionEvents.size());

        for (PhysicsCollisionEvent& ev : e->CollisionEvents)
        {
            MonoObject* col1 = componentDatabase.TryRetrieveDerivedObject(ev.Collider1, "Ouroboros", "Collider");
            MonoObject* col2 = componentDatabase.TryRetrieveDerivedObject(ev.Collider2, "Ouroboros", "Collider");

            if (col1 == nullptr || col2 == nullptr)
            {
                LOG_CORE_ERROR("ScriptSystem Error: CollisionEvent Broadcasted, but Collider involved not supported in C#");
                return;
            }

            MonoArray* arr = ScriptEngine::CreateArray(contactPointClass, ev.ContactCount);
            TempContactPoint temp;
            for (size_t i = 0; i < ev.ContactCount; ++i)
            {
                temp.normal = ScriptValue::vec3_type{ ev.ContactPoints[i].Normal.x, ev.ContactPoints[i].Normal.y, ev.ContactPoints[i].Normal.z };
                temp.point = ScriptValue::vec3_type{ ev.ContactPoints[i].Point.x, ev.ContactPoints[i].Point.y, ev.ContactPoints[i].Point.z };
                temp.impulse = ScriptValue::vec3_type{ ev.ContactPoints[i].Impulse.x, ev.ContactPoints[i].Impulse.y, ev.ContactPoints[i].Impulse.z };
                mono_array_set(arr, TempContactPoint, i, temp);
            }

            // Collision Data for Collider1

            MonoObject* collisionData1 = ScriptEngine::CreateObject(dataClass);
            mono_field_set_value(collisionData1, dataClassGameObjectField, componentDatabase.TryRetrieveGameObjectObject(ev.Collider1));
            mono_field_set_value(collisionData1, dataClassRigidbodyField, componentDatabase.TryRetrieveDerivedObject(ev.Collider1, "Ouroboros", "Rigidbody"));
            mono_field_set_value(collisionData1, dataClassColliderField, col1);
            mono_field_set_value(collisionData1, dataClassContactsField, arr);

            // Collision Data for Collider2

            MonoObject* collisionData2 = ScriptEngine::CreateObject(dataClass);
            mono_field_set_value(collisionData2, dataClassGameObjectField, componentDatabase.TryRetrieveGameObjectObject(ev.Collider2));
            mono_field_set_value(collisionData2, dataClassRigidbodyField, componentDatabase.TryRetrieveDerivedObject(ev.Collider2, "Ouroboros", "Rigidbody"));
            mono_field_set_value(collisionData2, dataClassColliderField, col2);
            mono_field_set_value(collisionData2, dataClassContactsField, arr);

            // set to map
            auto search1 = scriptDataMap.find(ev.Collider1);
            if (search1 == scriptDataMap.end())
                scriptDataMap.emplace(std::pair{ ev.Collider1, std::vector<CallbackData>{ { collisionData2, ev.State } } });
            else
                search1->second.emplace_back(CallbackData{ collisionData2, ev.State });

            auto search2 = scriptDataMap.find(ev.Collider2);
            if (search2 == scriptDataMap.end())
                scriptDataMap.emplace(std::pair{ ev.Collider2, std::vector<CallbackData>{ { collisionData1, ev.State } } });
            else
                search2->second.emplace_back(CallbackData{ collisionData1, ev.State });
        }

        //MonoMethod* onEnterMethod = nullptr;
        //MonoMethod* onStayMethod = nullptr;
        //MonoMethod* onExitMethod = nullptr;

        typedef void(__stdcall* CollisionFunction)(MonoObject*, MonoObject*, MonoException**);
        CollisionFunction onEnterThunk = nullptr;
        CollisionFunction onStayThunk = nullptr;
        CollisionFunction onExitThunk = nullptr;

        scriptDatabase.ForAllEnabledByClass([&scriptDataMap, &onEnterThunk, &onStayThunk, &onExitThunk](UUID uuid, MonoObject* script)
            {
                auto dataSearch = scriptDataMap.find(uuid);
                if (dataSearch == scriptDataMap.end())
                    return;
                std::vector<CallbackData>& callbackData = dataSearch->second;

                for (CallbackData& data : callbackData)
                {
                    //void* params[1];
                    //params[0] = data.data;
                    switch (data.state)
                    {
                    case PhysicsEventState::ENTER:
                    {
                        ScriptEngine::InvokeFunctionThunk(script, onEnterThunk, data.data);
                        //ScriptEngine::InvokeFunction(script, onEnterMethod, params);
                    }
                    break;
                    case PhysicsEventState::STAY:
                    {
                        ScriptEngine::InvokeFunctionThunk(script, onStayThunk, data.data);
                        //ScriptEngine::InvokeFunction(script, onStayMethod, params);
                    }
                    break;
                    case PhysicsEventState::EXIT:
                    {
                        ScriptEngine::InvokeFunctionThunk(script, onExitThunk, data.data);
                        //ScriptEngine::InvokeFunction(script, onExitMethod, params);
                    }
                    break;
                    }
                }
            }, [&onEnterThunk, &onStayThunk, &onExitThunk](MonoClass* klass)
            {
                MonoMethod* onEnterMethod = ScriptEngine::GetFunction(klass, "OnCollisionEnter", 1);
                onEnterThunk = (onEnterMethod == nullptr) ? nullptr : static_cast<CollisionFunction>(mono_method_get_unmanaged_thunk(onEnterMethod));
                MonoMethod* onStayMethod = ScriptEngine::GetFunction(klass, "OnCollisionStay", 1);
                onEnterThunk = (onStayMethod == nullptr) ? nullptr : static_cast<CollisionFunction>(mono_method_get_unmanaged_thunk(onStayMethod));
                MonoMethod* onExitMethod = ScriptEngine::GetFunction(klass, "OnCollisionExit", 1);
                onEnterThunk = (onExitMethod == nullptr) ? nullptr : static_cast<CollisionFunction>(mono_method_get_unmanaged_thunk(onExitMethod));
                return onEnterMethod != nullptr || onStayMethod != nullptr || onExitMethod != nullptr;
            });
    }
}