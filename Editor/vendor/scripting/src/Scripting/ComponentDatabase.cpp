/************************************************************************************//*!
\file           ComponentDatabase.cpp
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          defines the ComponentDatabase class, which acts as the main interface
                for creating, storing, and deleting instances of C# interfaces to C++
                ECS components belonging to each GameObject, as well as performing
                actions on a specified group of C# script instances

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "ComponentDatabase.h"

#include "ScriptEngine.h"

namespace oo
{
    std::unordered_map<std::string, ComponentDatabase::ComponentType> ComponentDatabase::componentTypeMap;

    void ComponentDatabase::RegisterComponent(std::string const& name_space, std::string const& name,
        ComponentAction Add, ComponentAction Remove, ComponentCheck Has,
        ComponentSetAction SetEnabled, ComponentCheck CheckEnabled)
    {
        std::string key = name_space + "." + name;
        componentTypeMap.emplace
        (
            key, ComponentType
            {
                Add, Remove, Has, SetEnabled, CheckEnabled, name_space, name, componentTypeMap.size()
            }
        );
    }

    size_t ComponentDatabase::GetComponentTypeIndex(const char* name_space, const char* name)
    {
        return GetComponentType(name_space, name).index;
    }

    ComponentDatabase::ComponentDatabase(SceneID sceneID) : sceneID{ sceneID }
    {

    }

    ComponentDatabase::~ComponentDatabase()
    {
        DeleteAll();
    }

    void ComponentDatabase::InstantiateObjectFull(UUID id)
    {
        objectMap.emplace(std::pair{ id, Object{} });

        Object& object = objectMap[id];
        object.componentList.resize(componentTypeMap.size());
        
        MonoClass* GOClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "GameObject");
        MonoObject* GO = ScriptEngine::CreateObject(GOClass);
        object.gameObject = mono_gchandle_new(GO, false);

        // set GameObject scene id
        MonoClassField* sceneField = mono_class_get_field_from_name(GOClass, "m_Scene");
        MonoClass* sceneClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Scene");
        MonoObject* sceneObj = ScriptEngine::CreateObject(sceneClass);
        MonoClassField* sceneIDField = mono_class_get_field_from_name(sceneClass, "m_SceneID");
        mono_field_set_value(sceneObj, sceneIDField, &sceneID);
        mono_field_set_value(GO, sceneField, mono_object_unbox(sceneObj));

        // set GameObject instance id
        MonoClassField* idField = mono_class_get_field_from_name(GOClass, "m_InstanceID");
        mono_field_set_value(GO, idField, &id);

        for (auto& [key, type] : componentTypeMap)
        {
            if (!type.Has(sceneID, id))
                continue;
            Instantiate(id, type.name_space.c_str(), type.name.c_str(), true);
        }

        // set GameObject's transform
        MonoObject* transform = ComponentDatabase::RetrieveObject(id, "Ouroboros", "Transform");
        MonoClassField* transformField = mono_class_get_field_from_name(GOClass, "m_Transform");
        mono_field_set_value(GO, transformField, transform);
    }

ComponentDatabase::IntPtr ComponentDatabase::Instantiate(UUID id, const char* name_space, const char* name, bool onlyScript)
    {
        IntPtr* component = TryGetComponent(id, name_space, name);
        if(component == nullptr)
            throw std::exception("ComponentDatabase: failed to find Object/Component Type");
        if (*component != 0)
            return *component;

        if (!onlyScript)
            GetComponentType(name_space, name).Add(sceneID, id);

        // create C# Component
        MonoClass* klass = ScriptEngine::GetClass("ScriptCore", name_space, name);
        MonoObject* object = ScriptEngine::CreateObject(klass);
        mono_runtime_object_init(object);
        *component = mono_gchandle_new(object, false);

        // set Component's gameObject
        MonoClassField* objField = mono_class_get_field_from_name(klass, "m_GameObject");
        MonoObject* gameObject = mono_gchandle_get_target(GetObject(id).gameObject);
        mono_field_set_value(object, objField, gameObject);

        // set Component's id
        MonoClassField* idField = mono_class_get_field_from_name(klass, "m_ComponentID");
        size_t typeID = GetComponentTypeIndex(name_space, name);
        mono_field_set_value(object, idField, &typeID);

        return *component;
    }

    bool ComponentDatabase::CheckExists(UUID id)
    {
        return objectMap.find(id) != objectMap.end();
    }

    bool ComponentDatabase::HasComponent(UUID id, const char* name_space, const char* name)
    {
        ComponentType& type = GetComponentType(name_space, name);
        return type.Has(sceneID, id);
    }

    ComponentDatabase::IntPtr ComponentDatabase::Retrieve(UUID id, const char* name_space, const char* name)
    {
        return GetComponent(id, name_space, name);
    }

    ComponentDatabase::IntPtr ComponentDatabase::TryRetrieve(UUID id, const char* name_space, const char* name)
    {
        IntPtr* ptr = TryGetComponent(id, name_space, name);
        if (ptr == nullptr)
            return 0;
        return *ptr;
    }

    ComponentDatabase::IntPtr ComponentDatabase::RetrieveGameObject(UUID id)
    {
        return GetObject(id).gameObject;
    }

    ComponentDatabase::IntPtr ComponentDatabase::TryRetrieveGameObject(UUID id)
    {
        Object* object = TryGetObject(id);
        if (object == nullptr)
            return 0;
        return object->gameObject;
    }

    bool ComponentDatabase::CheckEnabled(UUID id, const char* name_space, const char* name)
    {
        return GetComponentType(name_space, name).CheckEnabled(sceneID, id);
    }

    void ComponentDatabase::SetEnabled(UUID id, const char* name_space, const char* name, bool isEnabled)
    {
        GetComponentType(name_space, name).SetEnabled(sceneID, id, isEnabled);
    }

    void ComponentDatabase::Delete(UUID id, const char* name_space, const char* name, bool onlyScript)
    {
        IntPtr* component = TryGetComponent(id, name_space, name);
        if (component == nullptr || *component == 0)
            return;

        if (!onlyScript)
            GetComponentType(name_space, name).Remove(sceneID, id);
        mono_gchandle_free(*component);
        *component = 0;
    }

    void ComponentDatabase::Delete(UUID id)
    {
        Object* object = TryGetObject(id);
        if (object == nullptr)
            return;
        for (IntPtr& ptr : object->componentList)
        {
            if (ptr == 0)
                continue;
            mono_gchandle_free(ptr);
            ptr = 0;
        }
        mono_gchandle_free(object->gameObject);
        objectMap.erase(id);
    }

    void ComponentDatabase::DeleteAll()
    {
        for (auto& [uuid, object] : objectMap)
        {
            for (IntPtr ptr : object.componentList)
            {
                if (ptr == 0)
                    continue;
                mono_gchandle_free(ptr);
            }
            mono_gchandle_free(object.gameObject);
        }
        objectMap.clear();
    }

    ComponentDatabase::ComponentType& ComponentDatabase::GetComponentType(const char* name_space, const char* name)
    {
        std::string key = std::string{ name_space } + "." + name;
        auto search = componentTypeMap.find(key);
        if(search == componentTypeMap.end())
            throw std::exception((std::string{ "ComponentDatabase: component not registered: " } + key).c_str());
        return search->second;
    }

    ComponentDatabase::Object& ComponentDatabase::GetObject(UUID uuid)
    {
        auto search = objectMap.find(uuid);
        if (search == objectMap.end())
            throw std::exception("ComponentDatabase: failed to find object");
        return search->second;
    }

    ComponentDatabase::IntPtr& ComponentDatabase::GetComponent(Object& object, ComponentType& type)
    {
        return object.componentList[type.index];
    }

    ComponentDatabase::ComponentType* ComponentDatabase::TryGetComponentType(const char* name_space, const char* name)
    {
        std::string key = std::string{ name_space } + "." + name;
        auto search = componentTypeMap.find(key);
        if (search == componentTypeMap.end())
            return nullptr;
        return &(search->second);
    }

    ComponentDatabase::Object* ComponentDatabase::TryGetObject(UUID uuid)
    {
        auto search = objectMap.find(uuid);
        if (search == objectMap.end())
            return nullptr;
        return &(search->second);
    }

    ComponentDatabase::IntPtr* ComponentDatabase::TryGetComponent(Object& object, ComponentType& type)
    {
        return &(object.componentList[type.index]);
    }
}