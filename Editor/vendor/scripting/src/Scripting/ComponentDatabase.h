/************************************************************************************//*!
\file           ComponentDatabase.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          declares the ComponentDatabase class, which acts as the main interface
                for creating, storing, and deleting instances of C# interfaces to C++
                ECS components belonging to each GameObject, as well as performing
                actions on a specified group of C# script instances

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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

        static constexpr IntPtr InvalidPtr = 0;

    private:
        struct ComponentType
        {
            ComponentAction Add;
            ComponentAction Remove;
            ComponentCheck Has;

            std::string name_space;
            std::string name;
            size_t index;
        };
        static std::unordered_map<std::string, ComponentType> componentTypeMap;
        static std::unordered_map<std::string, std::vector<std::string>> inheritanceMap;

        struct Object
        {
            IntPtr gameObject; // C# GameObject
            std::vector<IntPtr> componentList; // list C# Components
        };
        std::unordered_map<UUID, Object> objectMap;
        SceneID sceneID;

    public:
        static void RegisterComponent(std::string const& name_space, std::string const& name,
            ComponentAction Add, ComponentAction Remove, ComponentCheck Has);

        static size_t GetComponentTypeIndex(const char* name_space, const char* name);

    public:
        ComponentDatabase(SceneID sceneID);
        ~ComponentDatabase();

        void Initialize();

        void InstantiateObjectFull(UUID id);
        IntPtr Instantiate(UUID id, const char* name_space, const char* name, bool onlyScript = false);

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
            if (ptr == InvalidPtr)
                return nullptr;
            return mono_gchandle_get_target(ptr);
        }
        IntPtr TryRetrieveDerived(UUID id, const char* name_space, const char* name);
        inline MonoObject* TryRetrieveDerivedObject(UUID id, const char* name_space, const char* name)
        {
            IntPtr ptr = TryRetrieveDerived(id, name_space, name);
            if (ptr == InvalidPtr)
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
            if (ptr == InvalidPtr)
                return nullptr;
            return mono_gchandle_get_target(ptr);
        }

        void Delete(UUID id, const char* name_space, const char* name, bool onlyScript = false);
        void Delete(UUID id);
        void DeleteAll();

    private:
        static ComponentType& GetComponentType(const char* key);
        static ComponentType& GetComponentType(const char* name_space, const char* name);
        Object& GetObject(UUID uuid);
        IntPtr& GetComponent(Object& object, ComponentType& type);

        inline IntPtr& GetComponent(UUID uuid, const char* name_space, const char* name)
        {
            return GetComponent(GetObject(uuid), GetComponentType(name_space, name));
        }


        static ComponentType* TryGetComponentType(const char* key);
        static ComponentType* TryGetComponentType(const char* name_space, const char* name);
        Object* TryGetObject(UUID uuid);
        IntPtr* TryGetComponent(Object& object, ComponentType& type);

        IntPtr* TryGetComponent(UUID uuid, const char* name_space, const char* name);

        IntPtr* TryGetComponentDerived(UUID uuid, const char* name_space, const char* name);
    };
}