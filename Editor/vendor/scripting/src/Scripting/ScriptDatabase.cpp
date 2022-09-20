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
            return 0;
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

    ScriptDatabase::InstancePool& ScriptDatabase::GetInstancePool(const char* name_space, const char* name)
    {
        Index index = GetInstancePoolIndex(name_space, name);
        if(index == INDEX_NOTFOUND)
            throw std::exception((std::string{ "ScriptDatabase: no such script: " } + name_space + "." + name).c_str());
        return poolList[index];
    }

    ScriptDatabase::Instance& ScriptDatabase::GetInstance(UUID id, InstancePool& pool)
    {
        auto search = pool.find(id);
        if (search == pool.end())
            throw std::exception("ScriptDatabase: object does not have script instance");
        return search->second;
    }

    ScriptDatabase::InstancePool* ScriptDatabase::TryGetInstancePool(const char* name_space, const char* name)
    {
        Index index = GetInstancePoolIndex(name_space, name);
        if (index == INDEX_NOTFOUND)
            nullptr;
        return &(poolList[index]);
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
}