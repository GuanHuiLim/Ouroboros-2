#pragma once

#include <string>
#include <unordered_map>
#include <functional>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/threads.h>

namespace oo
{
    class ScriptDatabase
    {
    public:
        using UUID = uint64_t;
        using IntPtr = uint32_t;
        using Callback = std::function<void(MonoObject* object)>;
        using ObjectCheck = std::function<bool(UUID)>;

    private:
        struct Instance
        {
            IntPtr handle;
            bool enabled;

            Instance(IntPtr ptr) : handle{ ptr }, enabled{ true } {}
        };
        using InstancePool = std::unordered_map<UUID, Instance>;
        std::unordered_map<std::string, InstancePool> scriptMap;

    public:
        ScriptDatabase();
        ~ScriptDatabase();

        void Initialize(std::vector<MonoClass*> const& classList);

        MonoObject* Instantiate(UUID id, const char* name_space, const char* name);
        IntPtr Store(UUID id, MonoObject* object);

        IntPtr Retrieve(UUID id, const char* name_space, const char* name);
        inline MonoObject* RetrieveObject(UUID id, const char* name_space, const char* name)
        {
            return mono_gchandle_get_target(Retrieve(id, name_space, name));
        }
        IntPtr TryRetrieve(UUID id, const char* name_space, const char* name);
        inline MonoObject* TryRetrieveObject(UUID id, const char* name_space, const char* name)
        {
            IntPtr ptr = TryRetrieve(id, name_space, name);
            if (ptr == 0)
                return nullptr;
            return mono_gchandle_get_target(ptr);
        }

        bool CheckEnabled(UUID id, const char* name_space, const char* name);
        void SetEnabled(UUID id, const char* name_space, const char* name, bool isEnabled);

        void Delete(UUID id, const char* name_space, const char* name);
        void Delete(UUID id);
        void DeleteAll();

        void ForEach(const char* name_space, const char* name, Callback callback, ObjectCheck filter = nullptr);
        void ForEach(UUID id, Callback callback, ObjectCheck filter = nullptr);
        void ForAll(Callback callback, ObjectCheck filter = nullptr);

        void ForEachEnabled(const char* name_space, const char* name, Callback callback, ObjectCheck filter = nullptr);
        void ForEachEnabled(UUID id, Callback callback, ObjectCheck filter = nullptr);
        void ForAllEnabled(Callback callback, ObjectCheck filter = nullptr);

    private:
        InstancePool& GetInstancePool(const char* name_space, const char* name);
        Instance& GetInstance(UUID id, InstancePool& pool);

        inline Instance& GetInstance(UUID id, const char* name_space, const char* name)
        {
            InstancePool& scriptPool = GetInstancePool(name_space, name);
            return GetInstance(id, scriptPool);
        }

        InstancePool* TryGetInstancePool(const char* name_space, const char* name);
        Instance* TryGetInstance(UUID id, InstancePool& pool);

        inline Instance* TryGetInstance(UUID id, const char* name_space, const char* name)
        {
            InstancePool* scriptPool = TryGetInstancePool(name_space, name);
            if (scriptPool == nullptr)
                return nullptr;
            return TryGetInstance(id, *scriptPool);
        }
    };
}