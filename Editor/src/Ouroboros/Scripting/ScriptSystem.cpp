#include "pch.h"
#include "ScriptSystem.h"

#include <fstream>

#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/Scene/EditorController.h"

namespace oo
{

    // Global stuff that transfers between scenes

    SceneManager const* ScriptSystem::s_SceneManager;
    std::string ScriptSystem::s_BuildPath;
    std::string ScriptSystem::s_ProjectPath;

    bool ScriptSystem::s_IsPlaying = false;
    std::vector<ScriptClassInfo> ScriptSystem::s_ScriptList;

    void ScriptSystem::LoadProject(std::string const& buildPath, std::string const& projectPath)
    {
        s_BuildPath = buildPath;
        s_ProjectPath = projectPath;
        if(Compile())
            Load();
    }
    bool ScriptSystem::Compile()
    {
        if (s_IsPlaying)
        {
            LOG_WARN("Script Warning: you are currently in play mode");
            return false;
        }
        if (!std::filesystem::exists(s_ProjectPath))
        {
            LOG_ERROR("Script Compiling Error: Expected Scripting project at \"" + s_ProjectPath + "\" does not exist");
            return false;
        }
        try
        {
            ScriptEngine::Compile(s_ProjectPath, s_BuildPath + "warnings.log", s_BuildPath + "errors.log");
        }
        catch (std::exception const& e)
        {
            LOG_ERROR(e.what());
            return false;
        }
        DisplayWarnings();
        if (DisplayErrors())
            return false;
        LOG_CORE_TRACE("Script Compiling Successful");
        return true;
    }
    void ScriptSystem::Load()
    {
        // load all system info for later use
        std::string dllPath = s_BuildPath + "Scripting.dll";
        if (!std::filesystem::exists(dllPath))
        {
            LOG_WARN("No Scripting.dll to load");
            return;
        }
        try
        {
            ScriptEngine::Load(dllPath);
        }
        catch (std::exception const& e)
        {
            LOG_ERROR(e.what());
            return;
        }
        LOG_CORE_TRACE("Script Loading Successful");

        s_ScriptList.clear();

        MonoClass* monoBehaviour = ScriptEngine::TryGetClass("ScriptCore", "Ouroboros", "MonoBehaviour");
        if (monoBehaviour != nullptr)
        {
            std::vector<MonoClass*> classList = ScriptEngine::GetClassesByBaseClass("Scripting", monoBehaviour);
            for (MonoClass* klass : classList)
            {
                s_ScriptList.emplace_back(ScriptClassInfo{ mono_class_get_namespace(klass), mono_class_get_name(klass) });
            }
        }

        //if (refresh)
        //{
        //    ScriptSystem* ss = WorldManager::GetActiveWorld().TryGetSystem<ScriptSystem>();
        //    if (ss != nullptr)
        //        ss->RefreshScriptInfoAll();
        //}
    }

    bool ScriptSystem::DisplayWarnings()
    {
        std::ifstream ifsWarnings(s_BuildPath + "warnings.log");
        if (!ifsWarnings)
        {
            LOG_ERROR("Script Compiling Error: Warning log file not generated");
            return false;
        }
        std::string line;
        bool hasWarnings = false;
        while (std::getline(ifsWarnings, line))
        {
            LOG_WARN(line);
            hasWarnings = true;
        }
        ifsWarnings.close();
        return hasWarnings;
    }
    bool ScriptSystem::DisplayErrors()
    {
        std::ifstream ifsErrors(s_BuildPath + "errors.log");
        if (!ifsErrors)
        {
            LOG_ERROR("Script Compiling Error: Error log file not generated");
            return false;
        }
        std::string line;
        bool hasErrors = false;
        while (std::getline(ifsErrors, line))
        {
            LOG_ERROR(line);
            hasErrors = true;
        }
        ifsErrors.close();
        return hasErrors;
    }

    // Scene specific script stuff
    ScriptSystem::ScriptSystem(Scene& scene) : scene{ scene }, componentDatabase{ scene.GetID() }
    {
        EventManager::Subscribe<ScriptSystem, GameObjectComponent::OnEnableEvent>(this, &ScriptSystem::OnObjectEnabled);
        EventManager::Subscribe<ScriptSystem, GameObjectComponent::OnDisableEvent>(this, &ScriptSystem::OnObjectDisabled);
    }

    ScriptSystem::~ScriptSystem()
    {
        EventManager::Unsubscribe<ScriptSystem, GameObjectComponent::OnEnableEvent>(this, &ScriptSystem::OnObjectEnabled);
        EventManager::Unsubscribe<ScriptSystem, GameObjectComponent::OnDisableEvent>(this, &ScriptSystem::OnObjectDisabled);

        scriptDatabase.DeleteAll();
        componentDatabase.DeleteAll();
    }

    bool ScriptSystem::StartPlay()
    {
        if (s_IsPlaying)
            return false;
        if (!ScriptEngine::IsLoaded())
            return false;
        if (DisplayErrors())
            return false;

        MonoClass* monoBehaviour = ScriptEngine::TryGetClass("ScriptCore", "Ouroboros", "MonoBehaviour");
        if (monoBehaviour != nullptr)
            scriptDatabase.Initialize(ScriptEngine::GetClassesByBaseClass("Scripting", monoBehaviour));

        static Ecs::Query query = []()
        {
            Ecs::Query query;
            query.with<GameObjectComponent, ScriptComponent>().build();
            return query;
        }();

        s_IsPlaying = true;
        scene.GetWorld().for_each(query, [&](GameObjectComponent& gameObject, ScriptComponent& script)
            {
                if (gameObject.Id == GameObject::ROOTID)
                    return;

                // TESTING
                script.AddScriptInfo(ScriptClassInfo{ "", "TestClass" });
                //script.GetScriptInfo(ScriptClassInfo{ "", "TestClass" })->FindFieldInfo("testScript")->value = ScriptValue{ ScriptValue::component_type{ gameObject.Id, "", "TestClass", true }};
                //script.GetScriptInfo(ScriptClassInfo{ "", "TestClass" })->FindFieldInfo("testComponent")->value = ScriptValue{ ScriptValue::component_type{ gameObject.Id, "Ouroboros", "Transform", false } };

                SetUpObject(gameObject.Id, script);
            });
        UpdateAllScriptFieldsWithInfo();
        InvokeForAll("Awake");
        InvokeForAll("Start");
        return true;
    }

    void ScriptSystem::SetUpObject(UUID uuid)
    {
        std::shared_ptr<GameObject> gameObject = scene.FindWithInstanceID(uuid);
        ASSERT(gameObject == nullptr);
        SetUpObject(uuid, gameObject->GetComponent<ScriptComponent>());
    }
    void ScriptSystem::SetUpObject(UUID uuid, ScriptComponent const& script)
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
        if (!s_IsPlaying)
            return false;

        s_IsPlaying = false;
        scriptDatabase.DeleteAll();
        componentDatabase.DeleteAll();
        return true;
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

    void ScriptSystem::ResetScriptInfo(UUID uuid, ScriptComponent& script, ScriptClassInfo const& classInfo)
    {
        auto& scriptInfoMap = script.GetScriptInfoAll();
        auto search = scriptInfoMap.find(classInfo.ToString());
        if (search == scriptInfoMap.end())
            return;
        search->second.ResetFieldValues();
        if (s_IsPlaying)
            UpdateScriptFieldsWithInfo(uuid, script);
    }
    void ScriptSystem::RefreshScriptInfoAll()
    {
        static Ecs::Query query = []()
        {
            Ecs::Query query;
            query.with<GameObjectComponent, ScriptComponent>().build();
            return query;
        }();

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
        if (!s_IsPlaying)
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
        if (!s_IsPlaying)
            return 0;
        return scriptDatabase.TryRetrieve(uuid, name_space, name);
    }
    void ScriptSystem::RemoveScript(ScriptDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!s_IsPlaying)
            return;
        scriptDatabase.Delete(uuid, name_space, name);
    }
    void ScriptSystem::SetScriptEnabled(ScriptDatabase::UUID uuid, const char* name_space, const char* name, bool isEnabled)
    {
        if (!s_IsPlaying)
            return;
        scriptDatabase.SetEnabled(uuid, name_space, name, isEnabled);
        std::shared_ptr<GameObject> gameObject = scene.FindWithInstanceID(uuid);
        if (!gameObject->ActiveInHierarchy())
            return;
        if(isEnabled)
            InvokeForObject(uuid, "OnEnable");
        else
            InvokeForObject(uuid, "OnDisable");
    }
    bool ScriptSystem::CheckScriptEnabled(ScriptDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!s_IsPlaying)
            return false;
        return scriptDatabase.CheckEnabled(uuid, name_space, name);
    }

    ComponentDatabase::IntPtr ScriptSystem::AddComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!s_IsPlaying)
            return 0;
        return componentDatabase.Instantiate(uuid, name_space, name);
    }
    ComponentDatabase::IntPtr ScriptSystem::GetComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!s_IsPlaying)
            return 0;
        return componentDatabase.TryRetrieve(uuid, name_space, name);
    }
    void ScriptSystem::RemoveComponent(ComponentDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!s_IsPlaying)
            return;
        componentDatabase.Delete(uuid, name_space, name);
    }
    void ScriptSystem::SetComponentEnabled(ComponentDatabase::UUID uuid, const char* name_space, const char* name, bool isEnabled)
    {
        if (!s_IsPlaying)
            return;
        componentDatabase.SetEnabled(uuid, name_space, name, isEnabled);
    }
    bool ScriptSystem::CheckComponentEnabled(ComponentDatabase::UUID uuid, const char* name_space, const char* name)
    {
        if (!s_IsPlaying)
            return false;
        return componentDatabase.CheckEnabled(uuid, name_space, name);
    }

    ComponentDatabase::IntPtr ScriptSystem::GetGameObject(ComponentDatabase::UUID uuid)
    {
        if (!s_IsPlaying)
            return 0;
        return componentDatabase.RetrieveGameObject(uuid);
    }

    void ScriptSystem::InvokeForObject(UUID uuid, const char* functionName, int paramCount, void** params)
    {
        if (!ScriptEngine::IsLoaded())
            return;
        if (!s_IsPlaying)
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
    void ScriptSystem::InvokeForObjectEnabled(UUID uuid, const char* functionName, int paramCount, void** params)
    {
        if (!ScriptEngine::IsLoaded())
            return;
        if (!s_IsPlaying)
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
        if (!s_IsPlaying)
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
        if (!s_IsPlaying)
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
        if (!s_IsPlaying)
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
        if (!s_IsPlaying)
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
        static Ecs::Query query = []()
        {
            Ecs::Query query;
            query.with<GameObjectComponent, ScriptComponent>().build();
            return query;
        }();
        scene.GetWorld().for_each(query, [&](GameObjectComponent& gameObject, ScriptComponent& script)
            {
                if (gameObject.Id == GameObject::ROOTID)
                    return;
                UpdateScriptFieldsWithInfo(gameObject.Id, script);
            });
    }
    void ScriptSystem::UpdateScriptFieldsWithInfo(UUID uuid, ScriptComponent& script)
    {
        for (auto const& [key, scriptInfo] : script.GetScriptInfoAll())
        {
            MonoObject* scriptObj = scriptDatabase.RetrieveObject(uuid, scriptInfo.classInfo.name_space.c_str(), scriptInfo.classInfo.name.c_str());
            MonoClass* scriptClass = ScriptEngine::GetClass("Scripting", scriptInfo.classInfo.name_space.c_str(), scriptInfo.classInfo.name.c_str());
            for (auto const& [key, fieldInfo] : scriptInfo.fieldMap)
            {
                MonoClassField* field = mono_class_get_field_from_name(scriptClass, fieldInfo.name.c_str());
                ScriptValue::SetFieldValue(scriptObj, field, fieldInfo.value);
            }
        }
    }
}