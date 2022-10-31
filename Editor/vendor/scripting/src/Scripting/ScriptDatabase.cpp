/************************************************************************************//*!
\file           ScriptDatabase.cpp
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          defines the ScriptDatabase class, which acts as the main interface
                for creating, storing, and deleting instances of C# scripts belonging to
                each GameObject, as well as performing actions on a specified group of
                C# script instances

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "ScriptDatabase.h"

#include "ScriptEngine.h"

namespace oo
{
    ScriptDatabase::ScriptDatabase()
    {
    }

    ScriptDatabase::~ScriptDatabase()
    {
        DeleteAll();
    }

    void ScriptDatabase::Initialize(std::vector<MonoClass*> const& classList)
    {
        indexMap.clear();
        poolList.clear();
        for (MonoClass* klass : classList)
        {
            std::string key = std::string{ ScriptEngine::GetClassInfoNameSpace(klass) } + "." + ScriptEngine::GetClassInfoName(klass);
            poolList.emplace_back();
            indexMap.emplace(key, poolList.size() - 1UL);
        }
        MonoClass* monoBehaviour = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "MonoBehaviour");
        for (MonoClass* klass : classList)
        {
            Index index = GetInstancePoolIndex(mono_class_get_namespace(klass), mono_class_get_name(klass));
            MonoClass* parent = mono_class_get_parent(klass);
            while (parent != monoBehaviour)
            {
                Index parentIndex = GetInstancePoolIndex(mono_class_get_namespace(parent), mono_class_get_name(parent));
                inheritanceMap[parentIndex].emplace_back(index);
                parent = mono_class_get_parent(parent);
            }
        }
    }

    ScriptDatabase::IntPtr ScriptDatabase::Instantiate(UUID id, const char* name_space, const char* name)
    {
        InstancePool& scriptPool = GetInstancePool(name_space, name);
        auto search = scriptPool.find(id);
        if (search != scriptPool.end())
            return search->second.handle;

        MonoClass* klass = ScriptEngine::GetClass("Scripting", name_space, name);
        MonoObject* object = ScriptEngine::CreateObject(klass);
        mono_runtime_object_init(object);
        IntPtr ptr = mono_gchandle_new(object, false);
        scriptPool.insert(std::pair<UUID, Instance>(id, Instance{ ptr }));
        return ptr;
    }

    ScriptDatabase::IntPtr ScriptDatabase::Store(UUID id, MonoObject* object)
    {
        MonoClass* klass = mono_object_get_class(object);
        InstancePool& scriptPool = GetInstancePool(ScriptEngine::GetClassInfoNameSpace(klass).c_str(), ScriptEngine::GetClassInfoName(klass).c_str());
        auto search = scriptPool.find(id);
        if (search != scriptPool.end())
            return search->second.handle;

        IntPtr ptr = mono_gchandle_new(object, false);
        scriptPool.insert(std::pair<UUID, Instance>(id, Instance{ ptr }));
        return ptr;
    }

    ScriptDatabase::IntPtr ScriptDatabase::Retrieve(UUID id, const char* name_space, const char* name)
    {
        Instance& instance = GetInstance(id, name_space, name);
        return instance.handle;
    }

    ScriptDatabase::IntPtr ScriptDatabase::TryRetrieve(UUID id, const char* name_space, const char* name)
    {
        Instance* instance = TryGetInstance(id, name_space, name);
        if (instance == nullptr)
            return InvalidPtr;
        return instance->handle;
    }

    ScriptDatabase::IntPtr ScriptDatabase::TryRetrieveDerived(UUID id, const char* name_space, const char* name)
    {
        Index baseIndex = GetInstancePoolIndex(name_space, name);
        Instance* instance = TryGetInstanceDerived(id, baseIndex);
        if (instance == nullptr)
            return InvalidPtr;
        return instance->handle;
    }

    bool ScriptDatabase::CheckEnabled(UUID id, const char* name_space, const char* name)
    {
        Instance& instance = GetInstance(id, name_space, name);
        return instance.enabled;
    }

    void ScriptDatabase::SetEnabled(UUID id, const char* name_space, const char* name, bool isEnabled)
    {
        Instance& instance = GetInstance(id, name_space, name);
        instance.enabled = isEnabled;
    }

    void ScriptDatabase::Delete(UUID id, const char* name_space, const char* name)
    {
        InstancePool& scriptPool = GetInstancePool(name_space, name);
        auto search = scriptPool.find(id);
        if (search != scriptPool.end())
        {
            mono_gchandle_free(search->second.handle);
            scriptPool.erase(search);
        }
    }

    void ScriptDatabase::Delete(UUID id)
    {
        for (InstancePool& scriptPool : poolList)
        {
            auto search = scriptPool.find(id);
            if (search != scriptPool.end())
            {
                mono_gchandle_free(search->second.handle);
                scriptPool.erase(search);
            }
        }
    }

    void ScriptDatabase::DeleteAll()
    {
        for (InstancePool& scriptPool : poolList)
        {
            for (auto& [uuid, instance] : scriptPool)
            {
                mono_gchandle_free(instance.handle);
            }
            scriptPool.clear();
        }
    }

    void ScriptDatabase::ForEach(const char* name_space, const char* name, Callback callback, ObjectCheck filter)
    {
        InstancePool& scriptPool = GetInstancePool(name_space, name);
        for (auto& [uuid, instance] : scriptPool)
        {
            if (filter != nullptr && !filter(uuid))
                continue;
            MonoObject* object = mono_gchandle_get_target(instance.handle);
            callback(object);
        }
    }
    void ScriptDatabase::ForEachEnabled(const char* name_space, const char* name, Callback callback, ObjectCheck filter)
    {
        InstancePool& scriptPool = GetInstancePool(name_space, name);
        for (auto& [uuid, instance] : scriptPool)
        {
            if (!instance.enabled)
                continue;
            if (filter != nullptr && !filter(uuid))
                continue;
            MonoObject* object = mono_gchandle_get_target(instance.handle);
            callback(object);
        }
    }

    void ScriptDatabase::ForEach(UUID id, Callback callback, ObjectCheck filter)
    {
        if (filter && !filter(id))
            return;
        for (InstancePool& scriptPool : poolList)
        {
            Instance* instance = TryGetInstance(id, scriptPool);
            if (instance == nullptr)
                continue;
            MonoObject* object = mono_gchandle_get_target(instance->handle);
            callback(object);
        }
    }
    void ScriptDatabase::ForEachEnabled(UUID id, Callback callback, ObjectCheck filter)
    {
        if (filter && !filter(id))
            return;
        for (InstancePool& scriptPool : poolList)
        {
            Instance* instance = TryGetInstance(id, scriptPool);
            if (instance == nullptr || !instance->enabled)
                continue;
            MonoObject* object = mono_gchandle_get_target(instance->handle);
            callback(object);
        }
    }

    void ScriptDatabase::ForAll(Callback callback, ObjectCheck filter)
    {
        for (InstancePool& scriptPool : poolList)
        {
            for (auto& [uuid, instance] : scriptPool)
            {
                if (filter && !filter(uuid))
                    continue;
                MonoObject* object = mono_gchandle_get_target(instance.handle);
                callback(object);
            }
        }
    }
    void ScriptDatabase::ForAllEnabled(Callback callback, ObjectCheck filter)
    {
        for (InstancePool& scriptPool : poolList)
        {
            for (auto& [uuid, instance] : scriptPool)
            {
                if (!instance.enabled)
                    continue;
                if (filter && !filter(uuid))
                    continue;
                MonoObject* object = mono_gchandle_get_target(instance.handle);
                callback(object);
            }
        }
    }

    ScriptDatabase::Index ScriptDatabase::GetInstancePoolIndex(const char* name_space, const char* name)
    {
        std::string scriptName = std::string{ name_space } + "." + name;
        auto search = indexMap.find(scriptName);
        if (search == indexMap.end())
            return INDEX_NOTFOUND;
        return search->second;
    }

    ScriptDatabase::Instance& ScriptDatabase::GetInstance(UUID id, InstancePool& pool)
    {
        auto search = pool.find(id);
        if (search == pool.end())
            throw std::exception("ScriptDatabase: object does not have script instance");
        return search->second;
    }

    ScriptDatabase::Instance* ScriptDatabase::TryGetInstance(UUID id, InstancePool& pool)
    {
        if (pool.size() <= 0)
            return nullptr;
        auto search = pool.find(id);
        if (search == pool.end())
            return nullptr;
        return &(search->second);
    }

    ScriptDatabase::Instance* ScriptDatabase::TryGetInstanceDerived(UUID id, Index baseIndex)
    {
        InstancePool* scriptPool = TryGetInstancePool(baseIndex);
        if (scriptPool != nullptr)
        {
            Instance* instance = TryGetInstance(id, *scriptPool);
            if (instance != nullptr)
                return instance;
        }

        auto potentialDerived = inheritanceMap.find(baseIndex);
        if (potentialDerived == inheritanceMap.end())
            return nullptr;
        for (Index derived : potentialDerived->second)
        {
            Instance* instance = TryGetInstanceDerived(id, derived);
            if (instance != nullptr)
                return instance;
        }
        return nullptr;
    }
}