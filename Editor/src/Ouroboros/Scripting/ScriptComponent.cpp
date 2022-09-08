#include "pch.h"
#include "ScriptComponent.h"

namespace oo
{
    //ScriptValue const Scripting::GetScriptValue(const char* name_space, const char* name, const char* fieldName)
    //{
    //    ScriptClassInfo classInfo{ name_space, name };
    //    utility::StringHash scriptKey{ classInfo.ToString() };
    //    auto const& infoSearch = scriptInfoMap.find(scriptKey);
    //    if (infoSearch == scriptInfoMap.end())
    //        return ScriptValue();

    //    MonoObject* scriptObj = ScriptDatabase::RetrieveObject(m_entity, name_space, name);
    //    MonoClass* scriptClass = mono_object_get_class(scriptObj);
    //    MonoClassField* field = mono_class_get_field_from_name(scriptClass, fieldName);

    //    auto const& fieldInfo = infoSearch->second.fieldMap.find(utility::StringHash{ fieldName })->second;

    //    return ScriptValue::GetFieldValue(scriptObj, field, fieldInfo.value);
    //}

    /*-----------------------------------------------------------------------------*/
    /* Script Info Functions                                                       */
    /*-----------------------------------------------------------------------------*/
    ScriptInfo& ScriptComponent::AddScriptInfo(ScriptClassInfo const& classInfo)
    {
        auto const& inserted = scriptInfoMap.emplace(classInfo.ToString(), ScriptInfo{ classInfo });
        return inserted.first->second;
    }

    ScriptInfo* ScriptComponent::GetScriptInfo(ScriptClassInfo const& classInfo)
    {
        auto const& search = scriptInfoMap.find(classInfo.ToString());
        if (search == scriptInfoMap.end())
            return nullptr;
        return &(search->second);
    }

    void ScriptComponent::RemoveScriptInfo(ScriptClassInfo const& classInfo)
    {
        scriptInfoMap.erase(classInfo.ToString());
    }

    ScriptComponent::map_type& ScriptComponent::GetScriptInfoAll()
    {
        return scriptInfoMap;
    }

    ScriptComponent::map_type const& ScriptComponent::GetScriptInfoAll() const
    {
        return scriptInfoMap;
    }
}