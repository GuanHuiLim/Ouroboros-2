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
    std::vector<std::string> ScriptSystem::s_ScriptList;

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
        //std::vector<MonoClass*> classList = ScriptEngine::GetClassesByBaseClass("Scripting", ScriptEngine::GetClass("ScriptCore", "Ouroboros", "MonoBehaviour"));
        //s_ClassList.clear();
        //std::vector<std::string> classNameList;
        //for (MonoClass* klass : classList)
        //{
        //    classNameList.emplace_back(std::string{ mono_class_get_namespace(klass) } + "." + mono_class_get_name(klass));
        //    s_ClassList.emplace_back(ScriptClassInfo{ mono_class_get_namespace(klass), mono_class_get_name(klass) });
        //}

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
        //EventManager::Subscribe<ScriptSystem, EditorController::OnSimulateEvent>(this, [](EditorController::OnSimulateEvent* e)
        //{

        //});
    }

    ScriptSystem::~ScriptSystem()
    {
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

        scriptDatabase.Initialize(ScriptEngine::GetClassesByBaseClass("Scripting", ScriptEngine::GetClass("ScriptCore", "Ouroboros", "MonoBehaviour")));

        Ecs::Query query;
        query.with<GameObjectComponent>().build();
        scene.GetWorld().for_each(query, [&](GameObjectComponent& gameObject)
            {
                if (gameObject.Id == 0)
                    return;
                componentDatabase.InstantiateObjectFull(gameObject.Id);

                // create script instance
                MonoObject* script = scriptDatabase.Instantiate(gameObject.Id, "", "TestClass");

                // set gameObject field
                MonoClass* klass = ScriptEngine::GetClass("Scripting", "", "TestClass");
                MonoClassField* gameObjectField = mono_class_get_field_from_name(klass, "m_GameObject");
                MonoObject* GO = componentDatabase.RetrieveGameObjectObject(gameObject.Id);
                mono_field_set_value(script, gameObjectField, GO);

                // set componentID field
                //unsigned int key = utility::StringHash(std::string(name_space) + "." + name);
                //MonoClassField* idField = mono_class_get_field_from_name(klass, "m_ComponentID");
                //mono_field_set_value(script, idField, &key);
            });
        s_IsPlaying = true;
        InvokeForAll("Awake");
        InvokeForAll("Start");
        return true;
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
                GameObject& object = *(scene.FindWithInstanceID(uuid));
                return object.ActiveInHierarchy();
            });
    }
}