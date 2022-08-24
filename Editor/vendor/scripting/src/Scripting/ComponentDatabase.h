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
        using SceneID = uint32_t;
        using IntPtr = uint32_t;
        using Callback = std::function<void(MonoObject* object)>;

        using ComponentAction = std::function<void(SceneID, UUID)>;
        using ComponentCheck = std::function<bool(SceneID, UUID)>;
        using ComponentSetAction = std::function<void(SceneID, UUID, bool)>;

    private:
        struct ComponentType
        {
            ComponentAction Add;
            ComponentAction Remove;
            ComponentCheck Has;
            ComponentSetAction SetEnabled;
            ComponentCheck CheckEnabled;

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
        std::vector<Object> objectMap;
        SceneID sceneID;

    public:
        static void RegisterComponent(std::string const& name_space, std::string const& name,
            ComponentAction Add, ComponentAction Remove, ComponentCheck Has,
            ComponentSetAction SetEnabled, ComponentCheck CheckEnabled);

        static size_t GetComponentTypeIndex(const char* name_space, const char* name);

    public:
        ComponentDatabase(SceneID sceneID);
        ~ComponentDatabase();

        void InstantiateObjectFull(UUID id);
        IntPtr Instantiate(UUID id, const char* name_space, const char* name, bool callAdd = false);

        bool CheckExists(UUID id);
        bool HasComponent(UUID id, const char* name_space, const char* name);

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
        IntPtr RetrieveGameObject(UUID id);
        inline MonoObject* RetrieveGameObjectObject(UUID id)
        {
            return mono_gchandle_get_target(RetrieveGameObject(id));
        }
        IntPtr TryRetrieveGameObject(UUID id);
        inline MonoObject* TryRetrieveGameObjectObject(UUID id)
        {
            IntPtr ptr = TryRetrieveGameObject(id);
            if (ptr == 0)
                return nullptr;
            return mono_gchandle_get_target(ptr);
        }

        bool CheckEnabled(UUID id, const char* name_space, const char* name);
        void SetEnabled(UUID id, const char* name_space, const char* name, bool isEnabled);

        void Delete(UUID id, const char* name_space, const char* name, bool callRemove = false);
        void Delete(UUID id);
        void DeleteAll();

    private:
        static ComponentType& GetComponentType(const char* name_space, const char* name);
        Object& GetObject(UUID uuid);
        IntPtr& GetComponent(Object& object, ComponentType& type);

        inline IntPtr& GetComponent(UUID uuid, const char* name_space, const char* name)
        {
            return GetComponent(GetObject(uuid), GetComponentType(name_space, name));
        }

        ComponentType* TryGetComponentType(const char* name_space, const char* name);
        Object* TryGetObject(UUID uuid);
        IntPtr* TryGetComponent(Object& object, ComponentType& type);

        inline IntPtr* TryGetComponent(UUID uuid, const char* name_space, const char* name)
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