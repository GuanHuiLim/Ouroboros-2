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
        scriptMap.clear();
        for (MonoClass* klass : classList)
        {
            std::string key = std::string(mono_class_get_namespace(klass)) + "." + mono_class_get_name(klass);
            scriptMap.insert(std::pair<std::string, InstancePool>(key, InstancePool{}));
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
        InstancePool& scriptPool = GetInstancePool(mono_class_get_namespace(klass), mono_class_get_name(klass));
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
        for (auto& [key, scriptPool] : scriptMap)
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
        for (auto& [key, scriptPool] : scriptMap)
        {
            for (auto& [uuid, instance] : scriptPool)
            {
                mono_gchandle_free(instance.handle);
            }
        }
        scriptMap.clear();
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
        for (auto& [key, scriptPool] : scriptMap)
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
        for (auto& [key, scriptPool] : scriptMap)
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
        for (auto& [key, scriptPool] : scriptMap)
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
        for (auto& [key, scriptPool] : scriptMap)
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

    ScriptDatabase::InstancePool& ScriptDatabase::GetInstancePool(const char* name_space, const char* name)
    {
        std::string key = std::string(name_space) + "." + name;
        auto search = scriptMap.find(key);
        if (search == scriptMap.end())
            throw std::exception((std::string{ "ScriptDatabase: no such script: " } + key).c_str());
        return search->second;
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
        std::string key = std::string(name_space) + "." + name;
        auto search = scriptMap.find(key);
        if (search == scriptMap.end())
            return nullptr;
        return &(search->second);
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