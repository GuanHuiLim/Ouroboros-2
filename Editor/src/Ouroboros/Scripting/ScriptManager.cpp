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
#ifdef OO_EDITOR
        Compile(); // compile first, always
        Load(); // regardless if compile succeeds or fails, try to load any pre-existing/new scripting.dll
#else
        Load();
#endif
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

    void ScriptManager::InsertBeforeDefaultOrder(ScriptClassInfo const& classInfo)
    {
        if (std::find(s_BeforeDefaultOrder.begin(), s_BeforeDefaultOrder.end(), classInfo) != s_BeforeDefaultOrder.end())
            throw std::exception{ "script is already in before default order" };
        if (std::find(s_AfterDefaultOrder.begin(), s_AfterDefaultOrder.end(), classInfo) != s_AfterDefaultOrder.end())
            throw std::exception{ "script is already in before after order" };

        s_BeforeDefaultOrder.emplace_back(classInfo);
    }
    void ScriptManager::InsertBeforeDefaultOrder(ScriptClassInfo const& classInfo, size_t pos)
    {
        if (std::find(s_BeforeDefaultOrder.begin(), s_BeforeDefaultOrder.end(), classInfo) != s_BeforeDefaultOrder.end())
            throw std::exception{ "script is already in before default order" };
        if (std::find(s_AfterDefaultOrder.begin(), s_AfterDefaultOrder.end(), classInfo) != s_AfterDefaultOrder.end())
            throw std::exception{ "script is already in before after order" };

        if (pos > s_BeforeDefaultOrder.size())
            s_BeforeDefaultOrder.emplace_back(classInfo);
        else
            s_BeforeDefaultOrder.emplace(s_BeforeDefaultOrder.begin() + pos, classInfo);
    }
    void ScriptManager::InsertBeforeDefaultOrder(ScriptClassInfo const& classInfo, std::vector<ScriptClassInfo>::iterator iter)
    {
        if (std::find(s_BeforeDefaultOrder.begin(), s_BeforeDefaultOrder.end(), classInfo) != s_BeforeDefaultOrder.end())
            throw std::exception{ "script is already in before default order" };
        if (std::find(s_AfterDefaultOrder.begin(), s_AfterDefaultOrder.end(), classInfo) != s_AfterDefaultOrder.end())
            throw std::exception{ "script is already in before after order" };

        s_BeforeDefaultOrder.emplace(iter, classInfo);
    }
    void ScriptManager::RemoveBeforeDefaultOrder(ScriptClassInfo const& classInfo)
    {
        auto search = std::find(s_BeforeDefaultOrder.begin(), s_BeforeDefaultOrder.end(), classInfo);
        if (search == s_BeforeDefaultOrder.end())
            throw std::exception{ "removing element from after order that does not exist" };
        s_BeforeDefaultOrder.erase(search);
    }
    void ScriptManager::InsertAfterDefaultOrder(ScriptClassInfo const& classInfo)
    {
        if (std::find(s_BeforeDefaultOrder.begin(), s_BeforeDefaultOrder.end(), classInfo) != s_BeforeDefaultOrder.end())
            throw std::exception{ "script is already in before default order" };
        if (std::find(s_AfterDefaultOrder.begin(), s_AfterDefaultOrder.end(), classInfo) != s_AfterDefaultOrder.end())
            throw std::exception{ "script is already in before after order" };

        s_AfterDefaultOrder.emplace_back(classInfo);
    }
    void ScriptManager::InsertAfterDefaultOrder(ScriptClassInfo const& classInfo, size_t pos)
    {
        if (std::find(s_BeforeDefaultOrder.begin(), s_BeforeDefaultOrder.end(), classInfo) != s_BeforeDefaultOrder.end())
            throw std::exception{ "script is already in before default order" };
        if (std::find(s_AfterDefaultOrder.begin(), s_AfterDefaultOrder.end(), classInfo) != s_AfterDefaultOrder.end())
            throw std::exception{ "script is already in before after order" };

        if (pos > s_AfterDefaultOrder.size())
            s_AfterDefaultOrder.emplace_back(classInfo);
        else
            s_AfterDefaultOrder.emplace(s_AfterDefaultOrder.begin() + pos, classInfo);
    }
    void ScriptManager::InsertAfterDefaultOrder(ScriptClassInfo const& classInfo, std::vector<ScriptClassInfo>::iterator iter)
    {
        if (std::find(s_BeforeDefaultOrder.begin(), s_BeforeDefaultOrder.end(), classInfo) != s_BeforeDefaultOrder.end())
            throw std::exception{ "script is already in before default order" };
        if (std::find(s_AfterDefaultOrder.begin(), s_AfterDefaultOrder.end(), classInfo) != s_AfterDefaultOrder.end())
            throw std::exception{ "script is already in before after order" };

        s_AfterDefaultOrder.emplace(iter, classInfo);
    }
    void ScriptManager::RemoveAfterDefaultOrder(ScriptClassInfo const& classInfo)
    {
        auto search = std::find(s_AfterDefaultOrder.begin(), s_AfterDefaultOrder.end(), classInfo);
        if (search == s_AfterDefaultOrder.end())
            throw std::exception{ "removing element from after order that does not exist" };
        s_AfterDefaultOrder.erase(search);
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