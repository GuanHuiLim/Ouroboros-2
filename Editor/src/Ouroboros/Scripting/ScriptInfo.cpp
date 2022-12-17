/************************************************************************************//*!
\file           ScriptInfo.cpp
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Defines the functions declared in ScriptInfo.h to set and retrieve
                info of C# script instances that will be created during play mode

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "ScriptInfo.h"

//#include "../Asset/AssetsManager.h"
//#include "Scripting.h"

namespace oo
{
    // Helper Functions
    static bool GetFunctionParamInfo(std::vector<ScriptFieldInfo>& paramInfoList, MonoMethod* method, int maxParamCount = -1)
    {
        MonoArray* paramList = mono_param_get_objects(mono_domain_get(), method);
        size_t count = mono_array_length(paramList);

        // function has too many parameters
        if (maxParamCount >= 0 && count > (size_t)maxParamCount)
            return false;

        paramInfoList.resize(count);
        for (int i = 0; i < count; ++i)
        {
            MonoObject* paramObj = mono_array_get(paramList, MonoObject*, i);
            MonoClass* paramClass = mono_object_get_class(paramObj);

            // check if out
            MonoProperty* isOutProperty = mono_class_get_property_from_name(paramClass, "IsOut");
            MonoObject* isOutValue = mono_property_get_value(isOutProperty, paramObj, NULL, NULL);
            bool isOut = *((bool*)mono_object_unbox(isOutValue));
            if (isOut)
                return false;

            // get param name
            MonoProperty* nameProperty = mono_class_get_property_from_name(paramClass, "Name");
            MonoObject* nameValue = mono_property_get_value(nameProperty, paramObj, NULL, NULL);
            paramInfoList[i].name = mono_string_to_utf8(mono_object_to_string(nameValue, NULL));

            // get param type
            MonoProperty* typeProperty = mono_class_get_property_from_name(paramClass, "ParameterType");
            MonoReflectionType* typeValue = (MonoReflectionType*)mono_property_get_value(typeProperty, paramObj, NULL, NULL);
            MonoClass* typeClass = mono_type_get_class(mono_reflection_type_get_type(typeValue));

            if (typeClass == mono_get_boolean_class())
            {
                paramInfoList[i].value = static_cast<ScriptValue>(false);
            }
            else if (typeClass == mono_get_int32_class())
            {
                paramInfoList[i].value = static_cast<ScriptValue>(0);
            }
            else if (typeClass == mono_get_single_class())
            {
                paramInfoList[i].value = static_cast<ScriptValue>(0.0f);
            }
            else if (typeClass == mono_get_string_class())
            {
                paramInfoList[i].value = static_cast<ScriptValue>(std::string(""));
            }
            else if (typeClass == ScriptEngine::GetClass("ScriptCore", "Ouroboros", "GameObject"))
            {
                paramInfoList[i].value = static_cast<ScriptValue>(UUID::Invalid);
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    /*-----------------------------------------------------------------------------*/
    /* ScriptFieldInfo                                                             */
    /*-----------------------------------------------------------------------------*/

    ScriptValue ScriptFieldInfo::TryGetRuntimeValue()
    {
        if (scriptPtr == ScriptDatabase::InvalidPtr || scriptField == nullptr || value.GetValueType() == ScriptValue::type_enum::FUNCTION)
            return value;
        MonoObject* scriptObject = mono_gchandle_get_target(scriptPtr);
        if (scriptObject == nullptr)
            return value;
        return ScriptValue::GetFieldValue(scriptObject, scriptField, value);
    }

    void ScriptFieldInfo::TrySetRuntimeValue(ScriptValue const& newValue)
    {
        if (scriptPtr == ScriptDatabase::InvalidPtr || scriptField == nullptr || value.GetValueType() == ScriptValue::type_enum::FUNCTION)
        {
            value = newValue;
        }
        else
        {
            ScriptValue::SetFieldValue(mono_gchandle_get_target(scriptPtr), scriptField, newValue);
        }
    }

    void ScriptFieldInfo::SetScriptReference(MonoClassField* field, ScriptDatabase::IntPtr objPtr)
    {
        scriptPtr = objPtr;
        scriptField = field;
    }

    /*-----------------------------------------------------------------------------*/
    /* ScriptInfo                                                                  */
    /*-----------------------------------------------------------------------------*/

    ScriptInfo::ScriptInfo(ScriptClassInfo const& _classInfo) : classInfo(_classInfo)
    {
        if (!classInfo.IsValid())
            throw std::exception{ (std::string{ "(ScriptInfo) invalid class info: " } + _classInfo.ToString()).c_str() };
        std::vector<ScriptFieldInfo> fieldList = classInfo.GetScriptFieldInfoAll();
        for (int i = 0; i < fieldList.size(); ++i)
        {
            displayOrder.emplace_back(fieldList[i].name);
            fieldMap.insert({ fieldList[i].name, fieldList[i] });
        }
    }

    ScriptFieldInfo* ScriptInfo::FindFieldInfo(std::string const& fieldName)
    {
        auto const& search = fieldMap.find(fieldName);
        if(search == fieldMap.end())
            return nullptr;
        return &(search->second);
    }

    ScriptInfo& ScriptInfo::CopyFieldValues(ScriptInfo const& src)
    {
        for (auto& entry : fieldMap)
        {
            auto search = src.fieldMap.find(entry.first);
            if (search == src.fieldMap.end())
                continue;

            if (!entry.second.value.IsOverridable(search->second.value))
                continue;
            if (entry.second.value.GetValueType() == ScriptValue::type_enum::CLASS)
            {
                ScriptValue::class_type entryValue = entry.second.value.GetValue<ScriptValue::class_type>();
                ScriptValue::class_type searchValue = search->second.value.GetValue<ScriptValue::class_type>();
                for (ScriptFieldInfo& entryInfo : entryValue.infoList)
                {
                    for (ScriptFieldInfo const& searchInfo : searchValue.infoList)
                    {
                        if (entryInfo.name == searchInfo.name && entryInfo.value.IsOverridable(searchInfo.value))
                        {
                            entryInfo.value = searchInfo.value;
                            break;
                        }
                    }
                }
                entry.second.value.SetValue<ScriptValue::class_type>(entryValue);
            }
            else if (entry.second.value.GetValueType() == ScriptValue::type_enum::LIST)
            {
                ScriptValue::list_type entryList = entry.second.value.GetValue<ScriptValue::list_type>();
                ScriptValue::list_type searchList = search->second.value.GetValue<ScriptValue::list_type>();
                if (entryList.type == ScriptValue::type_enum::CLASS)
                {
                    ScriptClassInfo entryClass{ entryList.name_space, entryList.name };
                    for (ScriptValue searchValue : searchList.valueList)
                    {
                        ScriptValue::class_type entryValue;
                        entryValue.name_space = entryClass.name_space;
                        entryValue.name = entryClass.name;
                        entryValue.infoList = entryClass.GetScriptFieldInfoAll();
                        for (ScriptFieldInfo& entryInfo : entryValue.infoList)
                        {
                            for (ScriptFieldInfo const& srcInfo : searchValue.GetValue<ScriptValue::class_type>().infoList)
                            {
                                if (entryInfo.name == srcInfo.name && entryInfo.value.IsOverridable(srcInfo.value))
                                {
                                    entryInfo.value = srcInfo.value;
                                    break;
                                }
                            }
                        }
                        entryList.valueList.emplace_back(ScriptValue{ entryValue });
                    }
                    entry.second.value.SetValue<ScriptValue::list_type>(entryList);
                }
                else
                {
                    entry.second.value = search->second.value;
                }
            }
            else
            {
                entry.second.value = search->second.value;
            }
        }
        return *this;
    }

    void ScriptInfo::ResetFieldValues()
    {
        fieldMap.clear();
        std::vector<ScriptFieldInfo> fieldList = classInfo.GetScriptFieldInfoAll();
        for (int i = 0; i < fieldList.size(); ++i)
        {
            fieldMap.insert({ fieldList[i].name, fieldList[i] });
        }
    }

    /*-----------------------------------------------------------------------------*/
    /* ScriptClassInfo                                                             */
    /*-----------------------------------------------------------------------------*/
    ScriptClassInfo::ScriptClassInfo(std::string const& fullName)
    {
        size_t separator = fullName.find_last_of('.');
        if (separator == std::string::npos)
        {
            name_space = "";
            name = fullName;
        }
        else
        {
            name_space = fullName.substr(0, separator);
            name = fullName.substr(separator);
        }
    }

    ScriptClassInfo::ScriptClassInfo(MonoClass* klass)
        : name_space{ ScriptEngine::GetClassInfoNameSpace(klass) }, name{ ScriptEngine::GetClassInfoName(klass) }
    {
    }

    bool ScriptClassInfo::IsValid() const
    {
        return ScriptEngine::CheckClassExists("Scripting", name_space.c_str(), name.c_str());
    }

    ScriptValue::type_enum const ScriptClassInfo::GetScriptFieldType(std::string const& fieldName)
    {
        MonoClass* klass = ScriptEngine::GetClass("Scripting", name_space.c_str(), name.c_str());
        MonoClassField* field = mono_class_get_field_from_name(klass, fieldName.c_str());
        return ScriptValue::GetType(mono_field_get_type(field));
    }

    std::vector<ScriptFieldInfo> const ScriptClassInfo::GetScriptFieldInfoAll() const
    {
        MonoClass* monoBehaviour = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "MonoBehaviour");
        MonoClass* _class = ScriptEngine::GetClass("Scripting", name_space.c_str(), name.c_str());
        // create sample instance with the default values
        MonoObject* sample = ScriptEngine::CreateObject(_class);
        mono_runtime_object_init(sample);

        std::vector<ScriptFieldInfo> resultList;

        while (_class != nullptr && _class != monoBehaviour)
        {
            void* iter = NULL;
            MonoClassField* field = nullptr;
            while ((field = mono_class_get_fields(_class, &iter)) != nullptr)
            {
                if (!ScriptEngine::CheckClassFieldInspectorVisible(sample, field))
                    continue;
                std::string fieldName(mono_field_get_name(field));
                ScriptValue fieldValue = ScriptValue::GetFieldValue(sample, field);
                if (!fieldValue.IsNullType())
                {
                    resultList.push_back({ fieldName, fieldValue });
                }
            }
            _class = mono_class_get_parent(_class);
        }
        return resultList;
    }

    ScriptValue::function_info ScriptClassInfo::GetFunctionInfo(std::string const& functionName, int paramCount) const
    {
        MonoClass* klass = ScriptEngine::GetClass("Scripting", name_space.c_str(), name.c_str());
        if(klass == nullptr)
            return ScriptValue::function_info{};
        MonoMethod* method = mono_class_get_method_from_name(klass, functionName.c_str(), paramCount);
        if(method == nullptr)
            return ScriptValue::function_info{};
        std::vector<ScriptFieldInfo> paramInfoList;
        if (!GetFunctionParamInfo(paramInfoList, method))
            return ScriptValue::function_info{};
        return ScriptValue::function_info{ name_space, name, functionName, paramInfoList };
    }

    std::vector<ScriptValue::function_info> ScriptClassInfo::GetFunctionInfoAll(bool onlyPublic, int maxParamCount) const
    {
        std::vector<std::string> excludeList
        {
            ".ctor",
            "Awake",
            "Update",
            "OnEnable",
            "OnDisable",
            "OnTriggerEnter2D",
            "OnTriggerStay2D",
            "OnTriggerExit2D",
        };

        MonoClass* klass = ScriptEngine::GetClass("Scripting", name_space.c_str(), name.c_str());
        void* iter = NULL;
        MonoMethod* method = nullptr;
        std::vector<ScriptValue::function_info> functionList;
        while ((method = mono_class_get_methods(klass, &iter)) != nullptr)
        {
            std::string functionName = mono_method_get_name(method);
            // function is to be excluded
            if (excludeList.size() > 0 && std::find(excludeList.begin(), excludeList.end(), functionName) != excludeList.end())
                continue;
            if (ScriptEngine::CheckClassMethodStatic(klass, method) || !ScriptEngine::CheckClassMethodReturnVoid(klass, method))
                continue;
            if (onlyPublic && !ScriptEngine::CheckClassMethodPublic(klass, method))
                continue;

            std::vector<ScriptFieldInfo> paramInfoList;
            if (GetFunctionParamInfo(paramInfoList, method, maxParamCount))
            {
                functionList.emplace_back(ScriptValue::function_info{ ScriptEngine::GetClassInfoNameSpace(klass), ScriptEngine::GetClassInfoName(klass), functionName, paramInfoList });
            }
        }
        return functionList;
    }

    std::vector<ScriptClassInfo> const ScriptClassInfo::GetFieldGenericTypeParams(const char* fieldName) const
    {
        MonoClass* _class = ScriptEngine::GetClass("Scripting", name_space.c_str(), name.c_str());
        MonoClassField* field = mono_class_get_field_from_name(_class, fieldName);
        MonoType* type = mono_field_get_type(field);
        std::vector<MonoType*> genericsList = ScriptEngine::GetTypeGenericTypes(type);
        std::vector<ScriptClassInfo> resultList;
        std::string genericNameSpace;
        std::string genericName;
        for (unsigned int i = 0; i < genericsList.size(); ++i)
        {
            MonoClass* genericClass = mono_type_get_class(genericsList[i]);
            genericNameSpace = ScriptEngine::GetClassInfoNameSpace(genericClass);
            genericName = ScriptEngine::GetClassInfoName(genericClass);
            resultList.push_back({ genericNameSpace, genericName });
        }
        return resultList;
    }

    std::vector<std::string> const ScriptClassInfo::GetEnumFieldOptions(const char* fieldName) const
    {
        MonoClass* _class = ScriptEngine::GetClass("Scripting", name_space.c_str(), name.c_str());
        MonoClassField* field = mono_class_get_field_from_name(_class, fieldName);
        MonoType* type = mono_field_get_type(field);
        return ScriptEngine::GetEnumOptions(type);
    }

    bool ScriptClassInfo::operator==(ScriptClassInfo const& rhs) const
    {
        return name_space == rhs.name_space && name == rhs.name;
    }

    bool ScriptClassInfo::operator!=(ScriptClassInfo const& rhs) const
    {
        return name_space != rhs.name_space || name != rhs.name;
    }

    std::string const ScriptClassInfo::ToString() const
    {
        if (name_space.size() <= 0)
            return name;
        return name_space + "." + name;
    }
}