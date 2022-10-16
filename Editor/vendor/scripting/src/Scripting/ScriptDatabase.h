/************************************************************************************//*!
\file           ScriptDatabase.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          declares the ScriptDatabase class, which acts as the main interface
                for creating, storing, and deleting instances of C# scripts belonging to
                each GameObject, as well as performing actions on a specified group of
                C# script instances

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
    class ScriptDatabase
    {
    public:
        using UUID = uint64_t;
        using IntPtr = uint32_t;
        using Callback = std::function<void(MonoObject* object)>;
        using ObjectCheck = std::function<bool(UUID)>;

    private:
        using Index = size_t;
        static constexpr Index INDEX_NOTFOUND = std::numeric_limits<Index>::max();

        struct Instance
        {
            IntPtr handle;
            bool enabled;

            Instance(IntPtr ptr) : handle{ ptr }, enabled{ true } {}
        };
        using InstancePool = std::unordered_map<UUID, Instance>;
        std::unordered_map<std::string, Index> indexMap;
        std::vector<InstancePool> poolList;
        std::unordered_map<Index, std::vector<Index>> inheritanceMap;
        // std::unordered_map<std::string, InstancePool> scriptMap;

    public:
        ScriptDatabase();
        ~ScriptDatabase();

        void Initialize(std::vector<MonoClass*> const& classList);

        IntPtr Instantiate(UUID id, const char* name_space, const char* name);
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
        IntPtr TryRetrieveDerived(UUID id, const char* name_space, const char* name);
        inline MonoObject* TryRetrieveDerivedObject(UUID id, const char* name_space, const char* name)
        {
            IntPtr ptr = TryRetrieveDerived(id, name_space, name);
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
        Index GetInstancePoolIndex(const char* name_space, const char* name);

        inline InstancePool& GetInstancePool(Index index)
        {
            if (index == INDEX_NOTFOUND)
                throw std::exception{ "ScriptDatabase GetInstancePool: no such script" };
            return poolList[index];
        }
        inline InstancePool& GetInstancePool(const char* name_space, const char* name)
        {
            Index index = GetInstancePoolIndex(name_space, name);
            return GetInstancePool(index);
        }

        inline InstancePool* TryGetInstancePool(Index index)
        {
            if (index == INDEX_NOTFOUND)
                nullptr;
            return &(poolList[index]);
        }
        inline InstancePool* TryGetInstancePool(const char* name_space, const char* name)
        {
            Index index = GetInstancePoolIndex(name_space, name);
            return TryGetInstancePool(index);
        }

        Instance& GetInstance(UUID id, InstancePool& pool);

        inline Instance& GetInstance(UUID id, Index index)
        {
            InstancePool& scriptPool = GetInstancePool(index);
            return GetInstance(id, scriptPool);
        }
        inline Instance& GetInstance(UUID id, const char* name_space, const char* name)
        {
            InstancePool& scriptPool = GetInstancePool(name_space, name);
            return GetInstance(id, scriptPool);
        }

        Instance* TryGetInstance(UUID id, InstancePool& pool);

        inline Instance* TryGetInstance(UUID id, Index index)
        {
            InstancePool* scriptPool = TryGetInstancePool(index);
            if (scriptPool == nullptr)
                return nullptr;
            return TryGetInstance(id, *scriptPool);
        }
        inline Instance* TryGetInstance(UUID id, const char* name_space, const char* name)
        {
            InstancePool* scriptPool = TryGetInstancePool(name_space, name);
            if (scriptPool == nullptr)
                return nullptr;
            return TryGetInstance(id, *scriptPool);
        }

        Instance* TryGetInstanceDerived(UUID id, Index baseIndex);
    };
}