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
    class ComponentDatabase
    {
    public:
        using UUID = uint64_t;
        using IntPtr = uint32_t;
        using Callback = std::function<void(MonoObject* object)>;

        static void RegisterComponent(std::string const& name_space, std::string const& name,
            std::function<void(UUID)> Add, std::function<void(UUID)> Remove, std::function<bool(UUID)> Has,
            std::function<void(UUID, bool)> SetEnabled, std::function<bool(UUID)> CheckEnabled);

        static size_t GetComponentTypeIndex(const char* name_space, const char* name);

        static void InstantiateObjectFull(UUID id);
        static IntPtr Instantiate(UUID id, const char* name_space, const char* name, bool callAdd = false);

        static bool CheckExists(UUID id);
        static bool HasComponent(UUID id, const char* name_space, const char* name);

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
        static IntPtr RetrieveGameObject(UUID id);
        static inline MonoObject* RetrieveGameObjectObject(UUID id)
        {
            return mono_gchandle_get_target(RetrieveGameObject(id));
        }
        static IntPtr TryRetrieveGameObject(UUID id);
        static inline MonoObject* TryRetrieveGameObjectObject(UUID id)
        {
            IntPtr ptr = TryRetrieveGameObject(id);
            if (ptr == 0)
                return nullptr;
            return mono_gchandle_get_target(ptr);
        }

        static bool CheckEnabled(UUID id, const char* name_space, const char* name);
        static void SetEnabled(UUID id, const char* name_space, const char* name, bool isEnabled);

        static void Delete(UUID id, const char* name_space, const char* name, bool callRemove = false);
        static void Delete(UUID id);
        static void DeleteAll();

    private:
        struct ComponentType
        {
            std::function<void(UUID)> Add;
            std::function<void(UUID)> Remove;
            std::function<bool(UUID)> Has;
            std::function<void(UUID, bool)> SetEnabled;
            std::function<bool(UUID)> CheckEnabled;

            std::string name_space;
            std::string name;
            size_t index;
        };
        static std::unordered_map<std::string, ComponentType> componentTypeMap;

        struct Object
        {
            IntPtr gameObject;
            std::vector<IntPtr> componentList;
        };
        static std::vector<Object> objectMap;

    private:
        static ComponentType& GetComponentType(const char* name_space, const char* name);
        static Object& GetObject(UUID uuid);
        static IntPtr& GetComponent(Object& object, ComponentType& type);

        static inline IntPtr& GetComponent(UUID uuid, const char* name_space, const char* name)
        {
            return GetComponent(GetObject(uuid), GetComponentType(name_space, name));
        }

        static ComponentType* TryGetComponentType(const char* name_space, const char* name);
        static Object* TryGetObject(UUID uuid);
        static IntPtr* TryGetComponent(Object& object, ComponentType& type);

        static inline IntPtr* TryGetComponent(UUID uuid, const char* name_space, const char* name)
        {
            Object* object = TryGetObject(uuid);
            if (object == nullptr)
                return nullptr;
            ComponentType* type = TryGetComponentType(name_space, name);
            if (type == nullptr)
                return nullptr;
            return TryGetComponent(*object, *type);
        }
    };
}