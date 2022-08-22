#include "ComponentDatabase.h"

#include "ScriptEngine.h"

namespace oo
{
    std::unordered_map<std::string, ComponentDatabase::ComponentType> ComponentDatabase::componentTypeMap;
    std::vector<ComponentDatabase::Object> ComponentDatabase::objectMap;

    void ComponentDatabase::RegisterComponent(std::string const& name_space, std::string const& name,
        std::function<void(UUID)> Add, std::function<void(UUID)> Remove, std::function<bool(UUID)> Has,
        std::function<void(UUID, bool)> SetEnabled, std::function<bool(UUID)> CheckEnabled)
    {
        std::string key = name_space + "." + name;
        componentTypeMap.insert(std::pair<std::string, ComponentType>
        {   key, ComponentType
            {
                Add, Remove, Has, SetEnabled, CheckEnabled, name_space, name, componentTypeMap.size()
            }
        });
    }

    size_t ComponentDatabase::GetComponentTypeIndex(const char* name_space, const char* name)
    {
        return GetComponentType(name_space, name).index;
    }

    void ComponentDatabase::InstantiateObjectFull(UUID id)
    {
        if (id >= objectMap.size())
            objectMap.resize(id + 1);

        Object& object = objectMap[id];
        object.componentList.resize(componentTypeMap.size());
        
        MonoClass* GOClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "GameObject");
        MonoObject* GO = ScriptEngine::CreateObject(GOClass);
        object.gameObject = mono_gchandle_new(GO, false);

        for (auto& [key, type] : componentTypeMap)
        {
            if (!type.Has(id))
                continue;
            Instantiate(id, type.name_space.c_str(), type.name.c_str());
        }
    }

ComponentDatabase::IntPtr ComponentDatabase::Instantiate(UUID id, const char* name_space, const char* name, bool callAdd)
    {
        if (id >= objectMap.size() || objectMap[id].gameObject == 0)
            throw std::exception("ComponentDatabase: failed to find object");

        IntPtr& component = GetComponent(id, name_space, name);
        if (component != 0)
            return component;

        if (callAdd)
            GetComponentType(name_space, name).Add(id);

        // create C# Component
        MonoClass* klass = ScriptEngine::GetClass("ScriptCore", name_space, name);
        MonoObject* object = ScriptEngine::CreateObject(klass);
        mono_runtime_object_init(object);
        component = mono_gchandle_new(object, false);

        // set Component's gameObject
        MonoClassField* objField = mono_class_get_field_from_name(klass, "m_GameObject");
        MonoObject* gameObject = mono_gchandle_get_target(GetObject(id).gameObject);
        mono_field_set_value(object, objField, gameObject);

        // set Component's id
        MonoClassField* idField = mono_class_get_field_from_name(klass, "m_ComponentID");
        size_t typeID = GetComponentTypeIndex(name_space, name);
        mono_field_set_value(object, idField, &typeID);

        return component;
    }

    bool ComponentDatabase::CheckExists(UUID id)
    {
        return id < objectMap.size() && objectMap[id].gameObject != 0;
    }

    bool ComponentDatabase::HasComponent(UUID id, const char* name_space, const char* name)
    {
        ComponentType& type = GetComponentType(name_space, name);
        return type.Has(id);
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
        return GetComponentType(name_space, name).CheckEnabled(id);
    }

    void ComponentDatabase::SetEnabled(UUID id, const char* name_space, const char* name, bool isEnabled)
    {
        GetComponentType(name_space, name).SetEnabled(id, isEnabled);
    }

    void ComponentDatabase::Delete(UUID id, const char* name_space, const char* name, bool callRemove)
    {
        IntPtr* component = TryGetComponent(id, name_space, name);
        if (component == nullptr || *component == 0)
            return;

        if (callRemove)
            GetComponentType(name_space, name).Remove(id);
        mono_gchandle_free(*component);
        *component = 0;
    }

    void ComponentDatabase::Delete(UUID id)
    {
        if (id >= objectMap.size())
            return;

        Object& object = GetObject(id);
        if (object.gameObject == 0)
            return;
        for (IntPtr& ptr : object.componentList)
        {
            if (ptr == 0)
                continue;
            mono_gchandle_free(ptr);
            ptr = 0;
        }
        mono_gchandle_free(object.gameObject);
        object.gameObject = 0;
    }

    void ComponentDatabase::DeleteAll()
    {
        for (Object& object : objectMap)
        {
            if (object.gameObject == 0)
                continue;
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
        if(uuid >= objectMap.size() || objectMap[uuid].gameObject == 0)
            throw std::exception("ComponentDatabase: failed to find object");
        return objectMap[uuid];
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
        if (uuid >= objectMap.size() || objectMap[uuid].gameObject == 0)
            return nullptr;
        return &(objectMap[uuid]);
    }

    ComponentDatabase::IntPtr* ComponentDatabase::TryGetComponent(Object& object, ComponentType& type)
    {
        return &(object.componentList[type.index]);
    }
}