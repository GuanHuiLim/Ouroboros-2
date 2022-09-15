#include "pch.h"
#include "ScriptManager.h"

#include <filesystem>
#include <fstream>

#include "ScriptSystem.h"

namespace oo
{
    // Global stuff that transfers between scenes

    SceneManager const* ScriptManager::s_SceneManager;
    std::string ScriptManager::s_BuildPath;
    std::string ScriptManager::s_ProjectPath;

    std::vector<ScriptClassInfo> ScriptManager::s_ScriptList;
    std::vector<ScriptClassInfo> ScriptManager::s_BeforeDefaultOrder;
    std::vector<ScriptClassInfo> ScriptManager::s_AfterDefaultOrder;

    void ScriptManager::LoadProject(std::string const& buildPath, std::string const& projectPath)
    {
        s_BuildPath = buildPath;
        s_ProjectPath = projectPath;
        if (Compile())
            Load();
    }
    bool ScriptManager::Compile()
    {
        if (s_SceneManager->HasActiveScene())
        {
            std::shared_ptr<Scene> scene = s_SceneManager->GetActiveScene<Scene>();
            ScriptSystem* scriptSystem = scene->GetWorld().Get_System<ScriptSystem>();
            if (scriptSystem != nullptr && scriptSystem->IsPlaying())
            {
                LOG_WARN("Script Warning: you are currently in play mode");
                return false;
            }
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
    void ScriptManager::Load()
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

    bool ScriptManager::DisplayWarnings()
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
    bool ScriptManager::DisplayErrors()
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

    std::vector<MonoClass*> const ScriptManager::GetScriptExecutionOrder()
    {
        std::vector<MonoClass*> executionOrder;
        
        MonoClass* monoBehaviour = ScriptEngine::TryGetClass("ScriptCore", "Ouroboros", "MonoBehaviour");
        if (monoBehaviour == nullptr)
            return executionOrder;

        std::vector<MonoClass*> baseClasses = ScriptEngine::GetClassesByBaseClass("Scripting", monoBehaviour);
        for (ScriptClassInfo const& classInfo : s_BeforeDefaultOrder)
        {
            MonoClass* klass = ScriptEngine::TryGetClass("Scripting", classInfo.name_space.c_str(), classInfo.name.c_str());
            if (klass == nullptr)
                continue;
            baseClasses.erase(std::find(baseClasses.begin(), baseClasses.end(), klass));
            executionOrder.emplace_back(klass);
        }
        std::vector<MonoClass*> afterExecutionOrder;
        for (ScriptClassInfo const& classInfo : s_AfterDefaultOrder)
        {
            MonoClass* klass = ScriptEngine::TryGetClass("Scripting", classInfo.name_space.c_str(), classInfo.name.c_str());
            if (klass == nullptr)
                continue;
            afterExecutionOrder.emplace_back(klass);
        }
        for (MonoClass* klass : baseClasses)
        {
            if (std::find(afterExecutionOrder.begin(), afterExecutionOrder.end(), klass) != afterExecutionOrder.end())
                continue;
            executionOrder.emplace_back(klass);
        }
        executionOrder.insert(executionOrder.end(), afterExecutionOrder.begin(), afterExecutionOrder.end());
        return executionOrder;
    }
}