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
        static ObjectCheck ObjectEnableCheck;

        struct Instance
        {
            IntPtr handle;
            bool enabled;

            Instance(IntPtr ptr) : handle{ ptr }, enabled{ true } {}
        };
        using InstancePool = std::unordered_map<UUID, Instance>;
        static std::unordered_map<std::string, InstancePool> scriptMap;

    public:
        static inline void SetMainCallbacks(ObjectCheck EnableCheck)
        {
            ObjectEnableCheck = EnableCheck;
        }
        static void Initialize(std::vector<MonoClass*> const& classList);
        static void Initialize(std::vector<std::string> const& classList);

        static MonoObject* Instantiate(UUID id, const char* name_space, const char* name);
        static IntPtr Store(UUID id, MonoObject* object);

        static IntPtr Retrieve(UUID id, const char* name_space, const char* name);
        static inline MonoObject* RetrieveObject(UUID id, const char* name_space, const char* name)
        {
            return mono_gchandle_get_target(Retrieve(id, name_space, name));
        }
        static IntPtr TryRetrieve(UUID id, const char* name_space, const char* name);
        static inline MonoObject* TryRetrieveObject(UUID id, const char* name_space, const char* name)
        {
            IntPtr ptr = TryRetrieve(id, name_space, name);
            if (ptr == 0)
                return nullptr;
            return mono_gchandle_get_target(ptr);
        }

        static bool CheckEnabled(UUID id, const char* name_space, const char* name);
        static void SetEnabled(UUID id, const char* name_space, const char* name, bool isEnabled);

        static void Delete(UUID id, const char* name_space, const char* name);
        static void Delete(UUID id);
        static void DeleteAll();

        static void ForEach(const char* name_space, const char* name, Callback callback, bool onlyEnabled = true);
        static void ForEach(UUID id, Callback callback, bool onlyEnabled = true);
        static void ForAll(Callback callback, bool onlyEnabled = true);

    private:
        static InstancePool& GetInstancePool(const char* name_space, const char* name);
        static Instance& GetInstance(UUID id, InstancePool& pool);

        static inline Instance& GetInstance(UUID id, const char* name_space, const char* name)
        {
            InstancePool& scriptPool = GetInstancePool(name_space, name);
            return GetInstance(id, scriptPool);
        }

        static InstancePool* TryGetInstancePool(const char* name_space, const char* name);
        static Instance* TryGetInstance(UUID id, InstancePool& pool);

        static inline Instance* TryGetInstance(UUID id, const char* name_space, const char* name)
        {
            InstancePool* scriptPool = TryGetInstancePool(name_space, name);
            if (scriptPool == nullptr)
                return nullptr;
            return TryGetInstance(id, *scriptPool);
        }
    };
}