/************************************************************************************//*!
\file           ScriptValue.cpp
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Declares the ScriptValue struct with the ability to store any supported
                C# type in C++ for serialization, and displaying/editing in the inspector.
                This is done by storing all the necessary information in C++ to recreate
                the C# type when needed. Helper functions are also provided to get a
                C# variable in the form of a ScriptValue, and to edit C# variables
                using a given ScriptValue

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "ScriptValue.h"
#include "ScriptInfo.h"
//#include "Scripting.h"

#include "ScriptManager.h"
#include "ScriptSystem.h"

namespace oo
{
    static void InvokeScriptFunctionValue(ScriptValue* value)
    {
        if (value == nullptr || !value->IsValueType<ScriptValue::function_type>())
            return;

        ScriptValue::function_type funcValue = value->GetValue<ScriptValue::function_type>();
        funcValue.Invoke();
    }

    std::map<ScriptValue::type_enum, ScriptValue::helper_functions> ScriptValue::utilityMap
    {
        {
            ScriptValue::type_enum::BOOL, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    bool fieldValue = value.GetValue<bool>();
                    mono_field_set_value(obj, field, &fieldValue);
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    bool fieldValue;
                    mono_field_get_value(object, field, &fieldValue);
                    return ScriptValue(fieldValue);
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    return ScriptValue(*(bool*)mono_object_unbox(object));
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);
                    bool entry = value.GetValue<bool>();
                    void* args[1];
                    args[0] = &entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        { 
            ScriptValue::type_enum::INT, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    int fieldValue = value.GetValue<int>();
                    mono_field_set_value(obj, field, &fieldValue);
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    int fieldValue;
                    mono_field_get_value(object, field, &fieldValue);
                    return ScriptValue(fieldValue);
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    return ScriptValue(*(int*)mono_object_unbox(object));
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);
                    int entry = value.GetValue<int>();
                    void* args[1];
                    args[0] = &entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        { 
            ScriptValue::type_enum::FLOAT, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    float fieldValue = value.GetValue<float>();
                    mono_field_set_value(obj, field, &fieldValue);
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    float fieldValue;
                    mono_field_get_value(object, field, &fieldValue);
                    return ScriptValue(fieldValue);
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    return ScriptValue(*(float*)mono_object_unbox(object));
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);
                    float entry = value.GetValue<float>();
                    void* args[1];
                    args[0] = &entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        { 
            ScriptValue::type_enum::STRING, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    MonoString* fieldValue = ScriptEngine::CreateString(value.GetValue<std::string>().c_str());
                    mono_field_set_value(obj, field, fieldValue);
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    MonoString* fieldValue;
                    mono_field_get_value(object, field, &fieldValue);
                    if (fieldValue == nullptr)
                        return ScriptValue(std::string());
                    return ScriptValue(std::string(mono_string_to_utf8(fieldValue)));
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    return ScriptValue(std::string(mono_string_to_utf8(mono_object_to_string(object, NULL))));
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);
                    MonoString* entry = ScriptEngine::CreateString(value.GetValue<std::string>().c_str());
                    void* args[1];
                    args[0] = entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        { 
            ScriptValue::type_enum::ENUM, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    ScriptValue::enum_type fieldValue = value.GetValue<ScriptValue::enum_type>();
                    mono_field_set_value(obj, field, &(fieldValue.index));
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    MonoType* type = mono_field_get_type(field);
                    MonoClass* enumClass = mono_type_get_class(type);
                    int fieldValue;
                    mono_field_get_value(object, field, &fieldValue);
                    return ScriptValue(ScriptValue::enum_type(ScriptEngine::GetClassInfoNameSpace(enumClass), ScriptEngine::GetClassInfoName(enumClass), fieldValue));
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    MonoClass* objClass = mono_object_get_class(object);
                    return ScriptValue(ScriptValue::enum_type(ScriptEngine::GetClassInfoNameSpace(objClass), ScriptEngine::GetClassInfoName(objClass), *(int*)mono_object_unbox(object)));
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);
                    int entry = value.GetValue<ScriptValue::enum_type>().index;
                    void* args[1];
                    args[0] = &entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        { 
            ScriptValue::type_enum::VECTOR2, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    vec2_type vec = value.GetValue<vec2_type>();
                    float xValue = vec.x;
                    float yValue = vec.y;
                    MonoClass* vecClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Vector2");
                    MonoObject* monoVec = ScriptEngine::CreateObject(vecClass);

                    MonoClassField* xField = mono_class_get_field_from_name(vecClass, "x");
                    mono_field_set_value(monoVec, xField, &xValue);
                    MonoClassField* yField = mono_class_get_field_from_name(vecClass, "y");
                    mono_field_set_value(monoVec, yField, &yValue);

                    mono_field_set_value(obj, field, mono_object_unbox(monoVec));
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    MonoType* type = mono_field_get_type(field);
                    MonoClass* typeClass = mono_type_get_class(type);
                    MonoObject* fieldObj = mono_field_get_value_object(mono_domain_get(), field, object);
                    float x = 0, y = 0;

                    if (fieldObj != nullptr)
                    {
                        MonoClassField* xField = mono_class_get_field_from_name(typeClass, "x");
                        mono_field_get_value(fieldObj, xField, &x);
                        MonoClassField* yField = mono_class_get_field_from_name(typeClass, "y");
                        mono_field_get_value(fieldObj, yField, &y);
                    }
                    return ScriptValue(vec2_type{ x, y });
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    float x = 0, y = 0;
                    if (object != nullptr)
                    {
                        MonoClass* objClass = mono_object_get_class(object);
                        MonoClassField* xField = mono_class_get_field_from_name(objClass, "x");
                        mono_field_get_value(object, xField, &x);
                        MonoClassField* yField = mono_class_get_field_from_name(objClass, "y");
                        mono_field_get_value(object, yField, &y);
                    }
                    return ScriptValue(vec2_type{ x, y });
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);

                    vec2_type vec2 = value.GetValue<vec2_type>();
                    MonoClass* klass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Vector2");
                    MonoObject* entry = ScriptEngine::CreateObject(klass);

                    MonoClassField* xField = mono_class_get_field_from_name(klass, "x");
                    mono_field_set_value(entry, xField, &vec2.x);
                    MonoClassField* yField = mono_class_get_field_from_name(klass, "y");
                    mono_field_set_value(entry, yField, &vec2.y);

                    void* args[1];
                    args[0] = entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        { 
            ScriptValue::type_enum::VECTOR3, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    vec3_type vec = value.GetValue<vec3_type>();
                    float xValue = vec.x;
                    float yValue = vec.y;
                    float zValue = vec.z;
                    MonoClass* vecClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Vector3");
                    MonoObject* monoVec = ScriptEngine::CreateObject(vecClass);

                    MonoClassField* xField = mono_class_get_field_from_name(vecClass, "x");
                    mono_field_set_value(monoVec, xField, &xValue);
                    MonoClassField* yField = mono_class_get_field_from_name(vecClass, "y");
                    mono_field_set_value(monoVec, yField, &yValue);
                    MonoClassField* zField = mono_class_get_field_from_name(vecClass, "z");
                    mono_field_set_value(monoVec, zField, &zValue);

                    mono_field_set_value(obj, field, mono_object_unbox(monoVec));
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    MonoType* type = mono_field_get_type(field);
                    MonoClass* typeClass = mono_type_get_class(type);
                    MonoObject* fieldObj = mono_field_get_value_object(mono_domain_get(), field, object);
                    float x = 0, y = 0, z = 0;

                    if (fieldObj != nullptr)
                    {
                        MonoClassField* xField = mono_class_get_field_from_name(typeClass, "x");
                        mono_field_get_value(fieldObj, xField, &x);
                        MonoClassField* yField = mono_class_get_field_from_name(typeClass, "y");
                        mono_field_get_value(fieldObj, yField, &y);
                        MonoClassField* zField = mono_class_get_field_from_name(typeClass, "z");
                        mono_field_get_value(fieldObj, zField, &z);
                    }
                    return ScriptValue(vec3_type{ x, y, z });
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    float x = 0, y = 0, z = 0;
                    if (object != nullptr)
                    {
                        MonoClass* objClass = mono_object_get_class(object);
                        MonoClassField* xField = mono_class_get_field_from_name(objClass, "x");
                        mono_field_get_value(object, xField, &x);
                        MonoClassField* yField = mono_class_get_field_from_name(objClass, "y");
                        mono_field_get_value(object, yField, &y);
                        MonoClassField* zField = mono_class_get_field_from_name(objClass, "z");
                        mono_field_get_value(object, zField, &z);
                    }
                    return ScriptValue(vec3_type{ x, y, z });
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);

                    vec3_type vec3 = value.GetValue<vec3_type>();
                    MonoClass* klass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Vector3");
                    MonoObject* entry = ScriptEngine::CreateObject(klass);

                    MonoClassField* xField = mono_class_get_field_from_name(klass, "x");
                    mono_field_set_value(entry, xField, &vec3.x);
                    MonoClassField* yField = mono_class_get_field_from_name(klass, "y");
                    mono_field_set_value(entry, yField, &vec3.y);
                    MonoClassField* zField = mono_class_get_field_from_name(klass, "z");
                    mono_field_set_value(entry, zField, &vec3.z);

                    void* args[1];
                    args[0] = entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        //{ 
        //    ScriptValue::type_enum::COLOUR, ScriptValue::helper_functions
        //    {
        //        // SetFieldValue
        //        [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
        //        {
        //            oo::Colour color = value.GetValue<oo::Colour>();
        //            float rValue = color.colour.r;
        //            float gValue = color.colour.g;
        //            float bValue = color.colour.b;
        //            float aValue = color.colour.a;
        //            MonoClass* colourClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Colour");
        //            MonoObject* monoColor = ScriptEngine::CreateObject(colourClass);

        //            MonoClassField* rField = mono_class_get_field_from_name(colourClass, "r");
        //            mono_field_set_value(monoColor, rField, &rValue);
        //            MonoClassField* gField = mono_class_get_field_from_name(colourClass, "g");
        //            mono_field_set_value(monoColor, gField, &gValue);
        //            MonoClassField* bField = mono_class_get_field_from_name(colourClass, "b");
        //            mono_field_set_value(monoColor, bField, &bValue);
        //            MonoClassField* aField = mono_class_get_field_from_name(colourClass, "a");
        //            mono_field_set_value(monoColor, aField, &aValue);

        //            mono_field_set_value(obj, field, mono_object_unbox(monoColor));
        //        },
        //        // GetFieldValue
        //        [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
        //        {
        //            MonoType* type = mono_field_get_type(field);
        //            MonoClass* typeClass = mono_type_get_class(type);
        //            MonoObject* fieldObj = mono_field_get_value_object(mono_domain_get(), field, object);
        //            float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;

        //            if (fieldObj != nullptr)
        //            {
        //                MonoClassField* colorField = mono_class_get_field_from_name(typeClass, "r");
        //                mono_field_get_value(fieldObj, colorField, &r);
        //                colorField = mono_class_get_field_from_name(typeClass, "g");
        //                mono_field_get_value(fieldObj, colorField, &g);
        //                colorField = mono_class_get_field_from_name(typeClass, "b");
        //                mono_field_get_value(fieldObj, colorField, &b);
        //                colorField = mono_class_get_field_from_name(typeClass, "a");
        //                mono_field_get_value(fieldObj, colorField, &a);
        //            }
        //            return ScriptValue(oo::Colour{ r, g, b, a });
        //        },
        //        // GetObjectValue
        //        [](MonoObject* object, ScriptValue const& refInfo)
        //        {
        //            float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
        //            if (object != nullptr)
        //            {
        //                MonoClass* objClass = mono_object_get_class(object);
        //                MonoClassField* rField = mono_class_get_field_from_name(objClass, "r");
        //                mono_field_get_value(object, rField, &r);
        //                MonoClassField* gField = mono_class_get_field_from_name(objClass, "g");
        //                mono_field_get_value(object, gField, &g);
        //                MonoClassField* bField = mono_class_get_field_from_name(objClass, "b");
        //                mono_field_get_value(object, bField, &b);
        //                MonoClassField* aField = mono_class_get_field_from_name(objClass, "a");
        //                mono_field_get_value(object, aField, &a);
        //            }
        //            return ScriptValue(oo::Colour{ r, g, b, a });
        //        },
        //        // AddToList
        //        [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
        //        {
        //            MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);

        //            oo::Colour colour = value.GetValue<oo::Colour>();
        //            MonoClass* klass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Colour");
        //            MonoObject* entry = ScriptEngine::CreateObject(klass);

        //            MonoClassField* rField = mono_class_get_field_from_name(klass, "r");
        //            mono_field_set_value(entry, rField, &colour.colour.r);
        //            MonoClassField* gField = mono_class_get_field_from_name(klass, "g");
        //            mono_field_set_value(entry, gField, &colour.colour.g);
        //            MonoClassField* bField = mono_class_get_field_from_name(klass, "b");
        //            mono_field_set_value(entry, bField, &colour.colour.b);
        //            MonoClassField* aField = mono_class_get_field_from_name(klass, "a");
        //            mono_field_set_value(entry, aField, &colour.colour.a);

        //            void* args[1];
        //            args[0] = entry;
        //            mono_runtime_invoke(addMethod, list, args, NULL);
        //        }
        //    }
        //},
        { 
            ScriptValue::type_enum::GAMEOBJECT, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    oo::UUID entityID = value.GetValue<oo::UUID>();
                    if (entityID == UUID::Invalid)
                    {
                        mono_field_set_value(obj, field, nullptr);
                        return;
                    }
                    std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
                    std::shared_ptr<GameObject> gameObject = scene->FindWithInstanceID(entityID);
                    if (gameObject == nullptr)
                    {
                        mono_field_set_value(obj, field, nullptr);
                        return;
                    }
                    ComponentDatabase::IntPtr ptr = scene->GetWorld().Get_System<ScriptSystem>()->GetGameObject(entityID);
                    if (ptr != ComponentDatabase::InvalidPtr)
                    {
                        MonoObject* monoGameObject = mono_gchandle_get_target(ptr);
                        mono_field_set_value(obj, field, monoGameObject);
                    }
                    else
                    {
                        mono_field_set_value(obj, field, nullptr);
                    }
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    MonoType* type = mono_field_get_type(field);
                    MonoClass* typeClass = mono_type_get_class(type);
                    MonoObject* fieldValue;
                    mono_field_get_value(object, field, &fieldValue);
                    if (fieldValue == nullptr)
                        return ScriptValue(UUID::Invalid);

                    MonoClassField* idField = mono_class_get_field_from_name(typeClass, "m_InstanceID");
                    UUID entityID;
                    mono_field_get_value(fieldValue, idField, &entityID);
                    std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
                    std::shared_ptr<GameObject> obj = scene->FindWithInstanceID(entityID);
                    if (obj == nullptr)
                        return ScriptValue(UUID::Invalid);
                    return ScriptValue(obj->GetInstanceID());
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    if (object == nullptr)
                        return ScriptValue(UUID::Invalid);

                    MonoClass* objClass = mono_object_get_class(object);
                    MonoClassField* idField = mono_class_get_field_from_name(objClass, "m_InstanceID");
                    UUID entityID;
                    mono_field_get_value(object, idField, &entityID);
                    std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
                    std::shared_ptr<GameObject> obj = scene->FindWithInstanceID(entityID);
                    if (obj == nullptr)
                        return ScriptValue(UUID::Invalid);
                    return ScriptValue(obj->GetInstanceID());
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);

                    MonoObject* entry = nullptr;
                    UUID entityID = value.GetValue<UUID>();
                    if (entityID != UUID::Invalid)
                    {
                        std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
                        std::shared_ptr<GameObject> obj = scene->FindWithInstanceID(entityID);
                        if (obj != nullptr)
                        {
                            ComponentDatabase::IntPtr ptr = scene->GetWorld().Get_System<ScriptSystem>()->GetGameObject(entityID);
                            entry = mono_gchandle_get_target(ptr);
                        }
                    }
                    void* args[1];
                    args[0] = entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        { 
            ScriptValue::type_enum::COMPONENT, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    ScriptValue::component_type component = value.GetValue<ScriptValue::component_type>();
                    std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
                    ScriptSystem* scriptSystem = scene->GetWorld().Get_System<ScriptSystem>();
                    ScriptDatabase::IntPtr ptr = ScriptDatabase::InvalidPtr;
                    if (component.m_isScript)
                        ptr = scriptSystem->GetScript(component.m_objID, component.m_namespace.c_str(), component.m_name.c_str());
                    else
                        ptr = scriptSystem->GetComponent(component.m_objID, component.m_namespace.c_str(), component.m_name.c_str());

                    MonoObject* monoComponent = nullptr;
                    if (ptr != ScriptDatabase::InvalidPtr)
                        monoComponent = mono_gchandle_get_target(ptr);
                    mono_field_set_value(obj, field, monoComponent);
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    MonoType* type = mono_field_get_type(field);
                    MonoClass* typeClass = mono_type_get_class(type);
                    UUID uuid = UUID::Invalid;
                    MonoObject* fieldValue;
                    mono_field_get_value(object, field, &fieldValue);
                    if (fieldValue != nullptr)
                    {
                        MonoClassField* objField = mono_class_get_field_from_name(typeClass, "m_GameObject");
                        MonoObject* compObj = mono_field_get_value_object(mono_domain_get(), objField, fieldValue);
                        MonoClassField* idField = mono_class_get_field_from_name(mono_object_get_class(compObj), "m_InstanceID");
                        UUID entityID;
                        mono_field_get_value(compObj, idField, &entityID);
                        std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
                        std::shared_ptr<GameObject> gameObject = scene->FindWithInstanceID(entityID);
                        if (gameObject != nullptr)
                        {
                            uuid = gameObject->GetInstanceID();
                        }
                    }
                    bool isScript = ScriptEngine::CheckClassInheritance(typeClass, ScriptEngine::GetClass("ScriptCore", "Ouroboros", "MonoBehaviour"));
                    std::string name_space{ ScriptEngine::GetClassInfoNameSpace(typeClass) };
                    std::string name{ ScriptEngine::GetClassInfoName(typeClass) };
                    return ScriptValue(ScriptValue::component_type{ uuid, name_space, name, isScript });
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    MonoClass* objClass = mono_object_get_class(object);
                    UUID uuid = UUID::Invalid;
                    if (object != nullptr)
                    {
                        MonoClassField* objField = mono_class_get_field_from_name(objClass, "m_GameObject");
                        MonoObject* compObj = mono_field_get_value_object(mono_domain_get(), objField, object);
                        MonoClassField* idField = mono_class_get_field_from_name(mono_object_get_class(compObj), "m_InstanceID");
                        UUID entityID;
                        mono_field_get_value(compObj, idField, &entityID);
                        std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
                        std::shared_ptr<GameObject> gameObject = scene->FindWithInstanceID(entityID);
                        if (gameObject != nullptr)
                        {
                            uuid = gameObject->GetInstanceID();
                        }
                    }
                    bool isScript = ScriptEngine::CheckClassInheritance(objClass, "ScriptCore", "Ouroboros", "MonoBehaviour");
                    std::string name_space{ ScriptEngine::GetClassInfoNameSpace(objClass) };
                    std::string name{ ScriptEngine::GetClassInfoName(objClass) };
                    return ScriptValue(ScriptValue::component_type{ uuid, name_space, name, isScript });
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);

                    ScriptValue::component_type component = value.GetValue<ScriptValue::component_type>();
                    std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
                    ScriptSystem* scriptSystem = scene->GetWorld().Get_System<ScriptSystem>();
                    ScriptDatabase::IntPtr ptr = ScriptDatabase::InvalidPtr;
                    if (component.m_isScript)
                        ptr = scriptSystem->GetScript(component.m_objID, component.m_namespace.c_str(), component.m_name.c_str());
                    else
                        ptr = scriptSystem->GetComponent(component.m_objID, component.m_namespace.c_str(), component.m_name.c_str());
                    MonoObject* entry = nullptr;
                    if (ptr != ScriptDatabase::InvalidPtr)
                        entry = mono_gchandle_get_target(ptr);

                    void* args[1];
                    args[0] = entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        //{
        //    ScriptValue::type_enum::ASSET, ScriptValue::helper_functions
        //    {
        //        // SetFieldValue
        //        [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
        //        {
        //            ScriptValue::asset_type asset = value.GetValue<ScriptValue::asset_type>();
        //            MonoObject* fieldValue = nullptr;
        //            MonoClass* fieldClass = nullptr;
        //            switch (asset.type)
        //            {
        //            case AssetType::Texture: fieldClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Texture"); break;
        //            case AssetType::AudioClip: fieldClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "AudioClip"); break;
        //            }
        //            if (fieldClass == nullptr)
        //                return;
        //            fieldValue = ScriptEngine::CreateObject(fieldClass);

        //            MonoClassField* handleField = mono_class_get_field_from_name(fieldClass, "assetHandle");
        //            mono_field_set_value(fieldValue, handleField, &(asset.handle));

        //            mono_field_set_value(obj, field, fieldValue);
        //        },
        //        // GetFieldValue
        //        [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
        //        {
        //            MonoType* type = mono_field_get_type(field);
        //            MonoClass* typeClass = mono_type_get_class(type);
        //            if (ScriptEngine::CheckClassInheritance(typeClass, "ScriptCore", "Ouroboros", "Texture")) // is a Texture
        //            {
        //                AssetType assetType = AssetType::Texture;
        //                AssetHandle handle = AssetManager::GetInternalAsset<Texture>("WhiteTexture")->GetHandle();

        //                MonoObject* fieldValue;
        //                mono_field_get_value(object, field, &fieldValue);
        //                if (fieldValue != nullptr)
        //                {
        //                    MonoClassField* handleField = mono_class_get_field_from_name(typeClass, "assetHandle");
        //                    mono_field_get_value(fieldValue, handleField, &handle);
        //                }
        //                return ScriptValue(ScriptValue::asset_type{ assetType, handle });
        //            }
        //            else if (ScriptEngine::CheckClassInheritance(typeClass, "ScriptCore", "Ouroboros", "AudioClip")) // is an AudioClip
        //            {
        //                AssetType assetType = AssetType::AudioClip;
        //                AssetHandle handle = 0;

        //                MonoObject* fieldValue;
        //                mono_field_get_value(object, field, &fieldValue);
        //                if (fieldValue != nullptr)
        //                {
        //                    MonoClassField* handleField = mono_class_get_field_from_name(typeClass, "assetHandle");
        //                    mono_field_get_value(fieldValue, handleField, &handle);
        //                }
        //                return ScriptValue(ScriptValue::asset_type{ assetType, handle });
        //            }
        //            return ScriptValue();
        //        },
        //        // GetObjectValue
        //        [](MonoObject* object, ScriptValue const& refInfo)
        //        {
        //            MonoClass* objClass = mono_object_get_class(object);
        //            if (ScriptEngine::CheckClassInheritance(objClass, "ScriptCore", "Ouroboros", "Texture")) // is a Texture
        //            {
        //                AssetType assetType = AssetType::Texture;
        //                AssetHandle handle = AssetManager::GetInternalAsset<Texture>("WhiteTexture")->GetHandle();

        //                if (object != nullptr)
        //                {
        //                    MonoClassField* handleField = mono_class_get_field_from_name(objClass, "assetHandle");
        //                    mono_field_get_value(object, handleField, &handle);
        //                }
        //                return ScriptValue(ScriptValue::asset_type{ assetType, handle });
        //            }
        //            else if (ScriptEngine::CheckClassInheritance(objClass, "ScriptCore", "Ouroboros", "AudioClip")) // is an AudioClip
        //            {
        //                AssetType assetType = AssetType::AudioClip;
        //                AssetHandle handle = 0;

        //                if (object != nullptr)
        //                {
        //                    MonoClassField* handleField = mono_class_get_field_from_name(objClass, "assetHandle");
        //                    mono_field_get_value(object, handleField, &handle);
        //                }
        //                return ScriptValue(ScriptValue::asset_type{ assetType, handle });
        //            }
        //            return ScriptValue();
        //        },
        //        // AddToList
        //        [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
        //        {
        //            MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);

        //            ScriptValue::asset_type asset = value.GetValue<ScriptValue::asset_type>();
        //            MonoObject* entry = nullptr;
        //            MonoClass* klass = nullptr;
        //            switch (asset.type)
        //            {
        //            case AssetType::Texture: klass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Texture"); break;
        //            case AssetType::AudioClip: klass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "AudioClip"); break;
        //            }
        //            if (klass != nullptr)
        //            {
        //                entry = ScriptEngine::CreateObject(klass);
        //                MonoClassField* handleField = mono_class_get_field_from_name(klass, "assetHandle");
        //                mono_field_set_value(entry, handleField, &(asset.handle));
        //            }

        //            void* args[1];
        //            args[0] = entry;
        //            mono_runtime_invoke(addMethod, list, args, NULL);
        //        }
        //    }
        //},
        //{ 
        //    ScriptValue::type_enum::PREFAB, ScriptValue::helper_functions
        //    {
        //        // SetFieldValue
        //        [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
        //        {
        //            ScriptValue::prefab_type prefab = value.GetValue<ScriptValue::prefab_type>();
        //            MonoClass* prefabClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Prefab");
        //            MonoObject* fieldValue = ScriptEngine::CreateObject(prefabClass);

        //            MonoString* pathString = ScriptEngine::CreateString(prefab.filePath.c_str());
        //            MonoClassField* pathField = mono_class_get_field_from_name(prefabClass, "filePath");
        //            mono_field_set_value(fieldValue, pathField, pathString);

        //            ScriptEngine::InvokeFunction(fieldValue, "LoadSourceObject");
        //            mono_field_set_value(obj, field, fieldValue);
        //        },
        //        // GetFieldValue
        //        [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
        //        {
        //            MonoType* type = mono_field_get_type(field);
        //            MonoClass* typeClass = mono_type_get_class(type);
        //            MonoObject* fieldObj = mono_field_get_value_object(mono_domain_get(), field, object);
        //            std::string filePath = "";
        //            if (fieldObj != nullptr)
        //            {
        //                MonoClassField* pathField = mono_class_get_field_from_name(typeClass, "filePath");
        //                MonoString* pathString;
        //                mono_field_get_value(fieldObj, pathField, &pathString);
        //                if (pathString != nullptr)
        //                {
        //                    filePath = mono_string_to_utf8(pathString);
        //                }
        //            }
        //            return ScriptValue(ScriptValue::prefab_type{ filePath });
        //        },
        //        // GetObjectValue
        //        [](MonoObject* object, ScriptValue const& refInfo)
        //        {
        //            std::string filePath = "";
        //            if (object != nullptr)
        //            {
        //                MonoClass* objClass = mono_object_get_class(object);
        //                MonoClassField* pathField = mono_class_get_field_from_name(objClass, "filePath");
        //                MonoString* pathString;
        //                mono_field_get_value(object, pathField, &pathString);
        //                if (pathString != nullptr)
        //                {
        //                    filePath = mono_string_to_utf8(pathString);
        //                }
        //            }
        //            return ScriptValue(ScriptValue::prefab_type{ filePath });
        //        },
        //        // AddToList
        //        [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
        //        {
        //            MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);

        //            ScriptValue::prefab_type prefab = value.GetValue<ScriptValue::prefab_type>();
        //            MonoClass* prefabClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "Prefab");
        //            MonoObject* entry = ScriptEngine::CreateObject(prefabClass);

        //            MonoString* pathString = ScriptEngine::CreateString(prefab.filePath.c_str());
        //            MonoClassField* pathField = mono_class_get_field_from_name(prefabClass, "filePath");
        //            mono_field_set_value(entry, pathField, pathString);
        //            ScriptEngine::InvokeFunction(entry, "LoadSourceObject");

        //            void* args[1];
        //            args[0] = entry;
        //            mono_runtime_invoke(addMethod, list, args, NULL);
        //        }
        //    }
        //},
        { 
            ScriptValue::type_enum::CLASS, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    MonoClass* classField = mono_type_get_class(mono_field_get_type(field));
                    MonoObject* fieldValue = ScriptEngine::CreateObject(classField);
                    std::vector<ScriptFieldInfo> const& fieldInfoList = value.GetValue<ScriptValue::class_type>().infoList;
                    for (ScriptFieldInfo const& fieldInfo : fieldInfoList)
                    {
                        MonoClassField* fieldValueField = mono_class_get_field_from_name(classField, fieldInfo.name.c_str());
                        ScriptValue::SetFieldValue(fieldValue, fieldValueField, fieldInfo.value);
                    }
                    mono_field_set_value(obj, field, fieldValue);
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    MonoType* type = mono_field_get_type(field);
                    MonoClass* typeClass = mono_type_get_class(type);
                    MonoObject* fieldValue;
                    mono_field_get_value(object, field, &fieldValue);
                    if (fieldValue == nullptr)
                    {
                        fieldValue = ScriptEngine::CreateObject(typeClass);
                        mono_runtime_object_init(fieldValue);
                    }

                    std::vector<ScriptFieldInfo> refFieldList;
                    if (refInfo.GetValueType() == type_enum::CLASS)
                        refFieldList = refInfo.GetValue<ScriptValue::class_type>().infoList;
                    size_t fieldIndex = 0;

                    std::vector<ScriptFieldInfo> resultList;
                    void* iter = NULL;
                    MonoClassField* valueField = nullptr;
                    while ((valueField = mono_class_get_fields(typeClass, &iter)) != nullptr)
                    {
                        if (!ScriptEngine::CheckClassFieldInspectorVisible(fieldValue, valueField))
                            continue;

                        std::string valueFieldName(mono_field_get_name(valueField));
                        ScriptValue valueFieldValue = ScriptValue::GetFieldValue(fieldValue, valueField,
                            ((fieldIndex < refFieldList.size()) ? refFieldList[fieldIndex].value : ScriptValue()));
                        ++fieldIndex;

                        resultList.push_back({ valueFieldName, valueFieldValue });
                    }
                    return ScriptValue(ScriptValue::class_type{ ScriptEngine::GetClassInfoNameSpace(typeClass), ScriptEngine::GetClassInfoName(typeClass), resultList });
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    MonoClass* objClass = mono_object_get_class(object);
                    if (object == nullptr)
                    {
                        object = ScriptEngine::CreateObject(objClass);
                        mono_runtime_object_init(object);
                    }

                    std::vector<ScriptFieldInfo> refFieldList;
                    if (refInfo.GetValueType() == type_enum::CLASS)
                        refFieldList = refInfo.GetValue<ScriptValue::class_type>().infoList;
                    size_t fieldIndex = 0;

                    std::vector<ScriptFieldInfo> resultList;
                    void* iter = NULL;
                    MonoClassField* valueField = nullptr;
                    while ((valueField = mono_class_get_fields(objClass, &iter)) != nullptr)
                    {
                        if (!ScriptEngine::CheckClassFieldInspectorVisible(object, valueField))
                            continue;
                        std::string valueFieldName(mono_field_get_name(valueField));
                        ScriptValue valueFieldValue = ScriptValue::GetFieldValue(object, valueField,
                            ((fieldIndex < refFieldList.size()) ? refFieldList[fieldIndex].value : ScriptValue()));
                        ++fieldIndex;

                        resultList.push_back({ valueFieldName, valueFieldValue });
                    }
                    return ScriptValue(ScriptValue::class_type{ ScriptEngine::GetClassInfoNameSpace(objClass), ScriptEngine::GetClassInfoName(objClass), resultList });
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);

                    MonoObject* entry = ScriptEngine::CreateObject(elementClass);
                    mono_runtime_object_init(entry);

                    std::vector<ScriptFieldInfo> const& fieldInfoList = value.GetValue<ScriptValue::class_type>().infoList;
                    for (ScriptFieldInfo const& fieldInfo : fieldInfoList)
                    {
                        MonoClassField* fieldValueField = mono_class_get_field_from_name(elementClass, fieldInfo.name.c_str());
                        ScriptValue::SetFieldValue(entry, fieldValueField, fieldInfo.value);
                    }

                    void* args[1];
                    args[0] = entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        { 
            ScriptValue::type_enum::LIST, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    MonoType* fieldType = mono_field_get_type(field);
                    MonoClass* valueClass = mono_type_get_class(ScriptEngine::GetTypeGenericTypes(fieldType)[0]);

                    MonoMethod* listCreateMethod = mono_class_get_method_from_name(ScriptEngine::GetClass("ScriptCore", "Ouroboros.Engine", "Utility"), "CreateInstanceOfGenericList", 2);
                    MonoClass* listClass = mono_class_from_name(mono_get_corlib(), "System.Collections.Generic", "List`1");
                    void* params[2];
                    params[0] = mono_type_get_object(mono_domain_get(), mono_class_get_type(listClass));
                    params[1] = mono_type_get_object(mono_domain_get(), mono_class_get_type(valueClass));
                    MonoObject* fieldValue = ScriptEngine::InvokeFunction(nullptr, listCreateMethod, params);

                    ScriptValue::list_type const& listValue = value.GetValue<ScriptValue::list_type>();
                    for (ScriptValue const& val : listValue.valueList)
                    {
                        ScriptValue::AddToList(fieldValue, valueClass, val);
                    }
                    mono_field_set_value(obj, field, fieldValue);
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    if (refInfo.GetValueType() == type_enum::LIST && refInfo.GetValue<ScriptValue::list_type>().type == type_enum::FUNCTION)
                        return refInfo;
                    MonoObject* fieldValue;
                    mono_field_get_value(object, field, &fieldValue);

                    MonoType* type = mono_field_get_type(field);
                    MonoClass* genericClass = mono_type_get_class(ScriptEngine::GetTypeGenericTypes(type)[0]);
                    type_enum genericValueType = GetType(mono_class_get_type(genericClass));

                    ScriptValue::list_type listValue{ genericValueType, ScriptEngine::GetClassInfoNameSpace(genericClass), ScriptEngine::GetClassInfoName(genericClass) };
                    if (fieldValue == nullptr)
                    {
                        return ScriptValue(listValue);
                    }

                    MonoClass* fieldClass = mono_object_get_class(fieldValue);
                    MonoProperty* countProperty = mono_class_get_property_from_name(fieldClass, "Count");
                    int count = *((int*)mono_object_unbox(mono_property_get_value(countProperty, fieldValue, NULL, NULL)));

                    MonoProperty* accessProperty = mono_class_get_property_from_name(fieldClass, "Item");
                    MonoMethod* accessMethod = mono_property_get_get_method(accessProperty);

                    std::vector<ScriptValue> refList;
                    if (refInfo.GetValueType() == type_enum::LIST)
                        refList = refInfo.GetValue<ScriptValue::list_type>().valueList;

                    int i = 0;
                    void* args[1];
                    args[0] = &i;
                    for (i = 0; i < count; ++i)
                    {
                        MonoObject* item = mono_runtime_invoke(accessMethod, fieldValue, args, NULL);
                        if (item != nullptr)
                        {
                            listValue.valueList.push_back
                            (
                                ScriptValue::GetObjectValue(item, (i < refList.size()) ? refList[i] : ScriptValue())
                            );
                        }
                        else
                            listValue.Push();
                    }
                    return ScriptValue(listValue);
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    MonoClass* objClass = mono_object_get_class(object);
                    MonoType* type = mono_class_get_type(objClass);

                    MonoClass* genericClass = mono_type_get_class(ScriptEngine::GetTypeGenericTypes(type)[0]);
                    type_enum genericValueType = GetType(mono_class_get_type(genericClass));

                    ScriptValue::list_type listValue{ genericValueType, ScriptEngine::GetClassInfoNameSpace(genericClass), ScriptEngine::GetClassInfoName(genericClass) };
                    if (object == nullptr)
                    {
                        return ScriptValue(listValue);
                    }

                    MonoProperty* countProperty = mono_class_get_property_from_name(objClass, "Count");
                    int count = *((int*)mono_object_unbox(mono_property_get_value(countProperty, object, NULL, NULL)));

                    MonoProperty* accessProperty = mono_class_get_property_from_name(objClass, "Item");
                    MonoMethod* accessMethod = mono_property_get_get_method(accessProperty);

                    int i = 0;
                    void* args[1];
                    args[0] = &i;
                    for (i = 0; i < count; ++i)
                    {
                        MonoObject* item = mono_runtime_invoke(accessMethod, object, args, NULL);
                        if (item != nullptr)
                            listValue.valueList.push_back(ScriptValue::GetObjectValue(item));
                        else
                            listValue.Push();
                    }
                    return ScriptValue(listValue);
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);

                    MonoType* fieldType = mono_class_get_type(elementClass);
                    MonoClass* valueClass = mono_type_get_class(ScriptEngine::GetTypeGenericTypes(fieldType)[0]);

                    MonoMethod* listCreateMethod = mono_class_get_method_from_name(ScriptEngine::GetClass("ScriptCore", "Ouroboros.Engine", "Utility"), "CreateInstanceOfGenericList", 2);
                    MonoClass* listClass = mono_class_from_name(mono_get_corlib(), "System.Collections.Generic", "List`1");
                    void* params[2];
                    params[0] = mono_type_get_object(mono_domain_get(), mono_class_get_type(listClass));
                    params[1] = mono_type_get_object(mono_domain_get(), mono_class_get_type(valueClass));
                    MonoObject* entry = ScriptEngine::InvokeFunction(nullptr, listCreateMethod, params);

                    ScriptValue::list_type const& listValue = value.GetValue<ScriptValue::list_type>();
                    for (ScriptValue const& val : listValue.valueList)
                    {
                        ScriptValue::AddToList(entry, valueClass, val);
                    }

                    void* args[1];
                    args[0] = entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
        { 
            ScriptValue::type_enum::FUNCTION, ScriptValue::helper_functions
            {
                // SetFieldValue
                [](MonoObject* obj, MonoClassField* field, ScriptValue const& value)
                {
                    MonoMethod* createMethod = mono_class_get_method_from_name(ScriptEngine::GetClass("ScriptCore", "Ouroboros.Engine", "Utility"), "GetButtonActionFromFunctionPointer", 2);
                    void* params[2];
                    intptr_t intPtr = reinterpret_cast<intptr_t>(InvokeScriptFunctionValue);
                    intptr_t paramPtr = reinterpret_cast<intptr_t>(&value);
                    params[0] = &intPtr;
                    params[1] = &paramPtr;
                    MonoObject* fieldValue = ScriptEngine::InvokeFunction(nullptr, createMethod, params);
                    mono_field_set_value(obj, field, fieldValue);
                },
                // GetFieldValue
                [](MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)
                {
                    if (refInfo.IsNullType())
                        return ScriptValue(ScriptValue::function_type{});
                    return refInfo;
                },
                // GetObjectValue
                [](MonoObject* object, ScriptValue const& refInfo)
                {
                    return ScriptValue(ScriptValue::function_type{});
                },
                // AddToList
                [](MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
                {
                    MonoMethod* addMethod = mono_class_get_method_from_name(mono_object_get_class(list), "Add", 1);

                    ScriptValue::function_type funcValue = value.GetValue<ScriptValue::function_type>();
                    MonoMethod* createMethod = mono_class_get_method_from_name(ScriptEngine::GetClass("ScriptCore", "Ouroboros.Engine", "Utility"), "GetButtonActionFromFunctionPointer", 2);
                    void* params[2];
                    intptr_t intPtr = reinterpret_cast<intptr_t>(InvokeScriptFunctionValue);
                    intptr_t paramPtr = reinterpret_cast<intptr_t>(&value);
                    params[0] = &intPtr;
                    params[1] = &paramPtr;
                    MonoObject* entry = ScriptEngine::InvokeFunction(nullptr, createMethod, params);

                    void* args[1];
                    args[0] = entry;
                    mono_runtime_invoke(addMethod, list, args, NULL);
                }
            }
        },
    };

    ScriptValue::helper_functions const& ScriptValue::GetHelper(type_enum type)
    {
        auto search = utilityMap.find(type);
        ASSERT(search == utilityMap.end());
        return search->second;
    }

    ScriptValue::type_enum ScriptValue::GetType(MonoType* type)
    {
        int typeEnum = mono_type_get_type(type);
        switch (typeEnum)
        {
        case MONO_TYPE_BOOLEAN: // bool
            return type_enum::BOOL;
        case MONO_TYPE_I4: // int
            return type_enum::INT;
        case MONO_TYPE_R4: // float
            return type_enum::FLOAT;
        case MONO_TYPE_STRING: // string
            return type_enum::STRING;
        case MONO_TYPE_VALUETYPE:
        {
            MonoClass* typeClass = mono_type_get_class(type);
            if (ScriptEngine::CheckClassInheritance(typeClass, mono_get_enum_class())) // Enum
                return type_enum::ENUM;
            if (ScriptEngine::CheckClassInheritance(typeClass, "ScriptCore", "Ouroboros", "Vector2")) // Vector2
                return type_enum::VECTOR2;
            if (ScriptEngine::CheckClassInheritance(typeClass, "ScriptCore", "Ouroboros", "Vector3")) // Vector3
                return type_enum::VECTOR3;
            //if (ScriptEngine::CheckClassInheritance(typeClass, "ScriptCore", "Ouroboros", "Colour")) // Colour
            //    return type_enum::COLOUR;
            return type_enum::EMPTY;
        }
        case MONO_TYPE_CLASS:
        {
            // for comparison
            MonoClass* typeClass = mono_type_get_class(type);
            MonoType* serializableType = mono_class_get_type(mono_class_from_name(mono_get_corlib(), "System", "SerializableAttribute"));

            if (mono_class_is_delegate(typeClass)) // is a delegate (function pointer)
            {
                MonoClass* actionClass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "ButtonAction");
                if (typeClass == actionClass) // only supported delegate, for inspector assigned functions
                    return type_enum::FUNCTION;
                return type_enum::EMPTY;
            }
            if (ScriptEngine::CheckClassInheritance(typeClass, "ScriptCore", "Ouroboros", "GameObject")) // is a GameObject
                return type_enum::GAMEOBJECT;
            if (ScriptEngine::CheckClassInheritance(typeClass, "ScriptCore", "Ouroboros", "Component")) // is a Component/Script
                return type_enum::COMPONENT;
            //if (ScriptEngine::CheckClassInheritance(typeClass, "ScriptCore", "Ouroboros", "Asset")) // is an Asset
            //    return type_enum::ASSET;
            //if (ScriptEngine::CheckClassInheritance(typeClass, "ScriptCore", "Ouroboros", "Prefab")) // is a Prefab
            //    return type_enum::PREFAB;
            if (ScriptEngine::CheckTypeHasAttribute(type, serializableType)) // is a container for info
                return type_enum::CLASS;
            return type_enum::EMPTY;
        }
        case MONO_TYPE_GENERICINST: // List
        {
            if (!ScriptEngine::CheckGenericList(type))
                return type_enum::EMPTY;
            MonoType* genericType = ScriptEngine::GetTypeGenericTypes(type)[0];
            if (ScriptEngine::CheckGenericList(genericType) || GetType(genericType) == type_enum::EMPTY)
                return type_enum::EMPTY;
            return type_enum::LIST;
        }
        }
        return type_enum::EMPTY;
    }

    bool ScriptValue::IsOverridable(ScriptValue const& src)
    {
        type_enum currType = GetValueType();
        type_enum srcType = src.GetValueType();

        if (currType != srcType)
            return false;
        if (currType == type_enum::ENUM)
        {
            enum_type currValue = GetValue<enum_type>();
            enum_type srcValue = src.GetValue<enum_type>();
            return currValue.name == srcValue.name && currValue.name_space == srcValue.name_space && srcValue.index < currValue.GetOptions().size();
        }
        if (currType == type_enum::COMPONENT)
        {
            component_type currValue = GetValue<component_type>();
            component_type srcValue = src.GetValue<component_type>();
            return currValue.m_isScript == srcValue.m_isScript && currValue.m_name == srcValue.m_name && currValue.m_namespace == srcValue.m_namespace;
        }
        //if (currType == type_enum::ASSET)
        //    return GetValue<asset_type>().type == src.GetValue<asset_type>().type;
        if (currType == type_enum::CLASS)
        {
            class_type currValue = GetValue<class_type>();
            class_type srcValue = src.GetValue<class_type>();
            return currValue.name == srcValue.name && currValue.name_space == srcValue.name_space;
        }
        if (currType == type_enum::LIST)
        {
            list_type currList = GetValue<list_type>();
            list_type srcList = src.GetValue<list_type>();
            return currList.type == srcList.type && currList.name == srcList.name && currList.name_space == srcList.name_space;
        }
        return true;
    }

    /*-----------------------------------------------------------------------------*/
    /* enum_type                                                                   */
    /*-----------------------------------------------------------------------------*/
    ScriptValue::enum_type::enum_type(std::string const& namespace_, std::string const& name_, unsigned int i) : name_space{ namespace_ }, name{ name_ }, index{ i }
    {
        if (index >= GetOptions().size())
            index = 0;
    };

    std::vector<std::string> ScriptValue::enum_type::GetOptions() const
    {
        MonoClass* klass = ScriptEngine::TryGetClass("Scripting", name_space.c_str(), name.c_str());
        if (klass == nullptr)
            klass = ScriptEngine::TryGetClass("ScriptCore", name_space.c_str(), name.c_str());
        ASSERT(klass == nullptr);
        return ScriptEngine::GetEnumOptions(mono_class_get_type(klass));
    }

    /*-----------------------------------------------------------------------------*/
    /* component_type                                                              */
    /*-----------------------------------------------------------------------------*/
    bool ScriptValue::component_type::is_valid()
    {
        if (m_objID == UUID::Invalid || m_namespace.size() <= 0 || m_name.size() <= 0)
            return false;

        std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
        std::shared_ptr<GameObject> obj = scene->FindWithInstanceID(m_objID);
        if (obj == nullptr)
            return false;

        if (m_isScript)
        {
            return obj->GetComponent<ScriptComponent>().GetScriptInfo(ScriptClassInfo{ m_namespace, m_name }) != nullptr;
        }
        else
        {
            return scene->GetWorld().Get_System<ScriptSystem>()->HasActualComponent(m_objID, m_namespace.c_str(), m_name.c_str());
        }
    }

    /*-----------------------------------------------------------------------------*/
    /* list_type                                                                   */
    /*-----------------------------------------------------------------------------*/
    void ScriptValue::list_type::Push()
    {
        switch (type)
        {
        case ScriptValue::type_enum::BOOL:
            valueList.emplace_back(false);
            break;
        case ScriptValue::type_enum::INT:
            valueList.emplace_back(0);
            break;
        case ScriptValue::type_enum::FLOAT:
            valueList.emplace_back(0.0f);
            break;
        case ScriptValue::type_enum::STRING:
            valueList.emplace_back(std::string{});
            break;
        case ScriptValue::type_enum::ENUM:
        {
            valueList.emplace_back(ScriptValue::enum_type{ name_space, name, 0 });
        }
        break;
        case ScriptValue::type_enum::VECTOR2:
            valueList.emplace_back(vec2_type{ 0, 0 });
            break;
        case ScriptValue::type_enum::VECTOR3:
            valueList.emplace_back(vec3_type{ 0, 0, 0 });
            break;
        //case ScriptValue::type_enum::COLOUR:
        //    valueList.emplace_back(oo::Colour{ 1, 1, 1, 1 });
        //    break;
        case ScriptValue::type_enum::GAMEOBJECT:
            valueList.emplace_back(UUID::Invalid);
            break;
        case ScriptValue::type_enum::COMPONENT:
        {
            MonoClass* klass = ScriptEngine::TryGetClass("ScriptCore", name_space.c_str(), name.c_str());
            if (klass == nullptr)
                klass = ScriptEngine::TryGetClass("Scripting", name_space.c_str(), name.c_str());
            valueList.emplace_back(
                ScriptValue::component_type
                {
                    UUID::Invalid,
                    name_space,
                    name,
                    ScriptEngine::CheckClassInheritance(klass, "ScriptCore", "Ouroboros", "MonoBehaviour")
                });
        }
        break;
        //case ScriptValue::type_enum::ASSET:
        //{
        //    MonoClass* klass = ScriptEngine::GetClass("ScriptCore", name_space.c_str(), name.c_str());
        //    if (klass == nullptr)
        //        klass = ScriptEngine::GetClass("Scripting", name_space.c_str(), name.c_str());
        //    if (ScriptEngine::CheckClassInheritance(klass, "ScriptCore", "Ouroboros", "Texture")) // is a Texture
        //    {
        //        AssetType assetType = AssetType::Texture;
        //        AssetHandle handle = AssetManager::GetInternalAsset<Texture>("WhiteTexture")->GetHandle();
        //        valueList.emplace_back(ScriptValue::asset_type{ assetType, handle });
        //    }
        //    else if (ScriptEngine::CheckClassInheritance(klass, "ScriptCore", "Ouroboros", "AudioClip")) // is an AudioClip
        //    {
        //        AssetType assetType = AssetType::AudioClip;
        //        AssetHandle handle = 0;
        //        valueList.emplace_back(ScriptValue::asset_type{ assetType, handle });
        //    }
        //}
        //break;
        //case ScriptValue::type_enum::PREFAB:
        //    valueList.emplace_back(ScriptValue::prefab_type{ "" });
        //    break;
        case ScriptValue::type_enum::CLASS:
        {
            ScriptClassInfo classInfo{ name_space, name };
            ScriptValue::class_type classValue;
            valueList.emplace_back(ScriptValue::class_type{ name_space, name, classInfo.GetScriptFieldInfoAll() });
        }
        break;
        case ScriptValue::type_enum::FUNCTION:
        {
            valueList.emplace_back(ScriptValue::function_type{});
        }
        break;
        }
    }

    void ScriptValue::list_type::Remove(size_t index)
    {
        valueList.erase(valueList.begin() + index);
    }

    /*-----------------------------------------------------------------------------*/
    /* ScriptFunctionInfo                                                          */
    /*-----------------------------------------------------------------------------*/
    void ScriptValue::function_info::Reset()
    {
        classNamespace.clear();
        className.clear();
        functionName.clear();
        paramList.clear();
    }

    void ScriptValue::function_info::Invoke(UUID uuid) const
    {
        if (className.size() <= 0 || functionName.size() <= 0)
        {
            LOG_WARN("ScriptFunctionInfo Invoke failed, function not set");
            return;
        }
        std::shared_ptr<Scene> scene = ScriptManager::s_SceneManager->GetActiveScene<Scene>();
        ScriptSystem* scriptSystem = scene->GetWorld().Get_System<ScriptSystem>();
        MonoObject* script = mono_gchandle_get_target(scriptSystem->GetScript(uuid, classNamespace.c_str(), className.c_str()));
        if (script == nullptr)
        {
            LOG_WARN("{0} does not have the {1}.{2} script", uuid, classNamespace, className);
            return;
        }
        if (paramList.size() <= 0)
        {
            try
            {
                ScriptEngine::InvokeFunction(script, functionName.c_str());
            }
            catch (std::exception const& e)
            {
                LOG_ERROR(e.what());
            }
            return;
        }
        std::vector<void*> ptrList(paramList.size());
        for (size_t i = 0; i < paramList.size(); ++i)
        {
            switch (paramList[i].value.GetValueType())
            {
            case ScriptValue::type_enum::STRING:
            {
                std::string const& str = paramList[i].value.GetValue<std::string>();
                MonoString* monoStr = ScriptEngine::CreateString(str.c_str());
                ptrList[i] = monoStr;
            }
            break;
            case ScriptValue::type_enum::GAMEOBJECT:
            {
                UUID paramUUID = paramList[i].value.GetValue<UUID>();
                if (paramUUID == UUID::Invalid) // id not set
                {
                    ptrList[i] = nullptr;
                    break;
                }
                std::shared_ptr<GameObject> objParam = scene->FindWithInstanceID(paramUUID);
                if (objParam == nullptr) // C++ object not found
                {
                    ptrList[i] = nullptr;
                    break;
                }
                ComponentDatabase::IntPtr objPtr = scriptSystem->GetGameObject(paramUUID);
                if (objPtr == ComponentDatabase::InvalidPtr) // C# object not found
                {
                    ptrList[i] = nullptr;
                    break;
                }
                ptrList[i] = mono_gchandle_get_target(objPtr);
            }
            break;
            default:
                ptrList[i] = (void*)(&paramList[i].value);
                break;
            }
        }
        try
        {
            ScriptEngine::InvokeFunction(script, functionName.c_str(), &(ptrList[0]), static_cast<int>(paramList.size()));
        }
        catch (std::exception const& e)
        {
            LOG_ERROR(e.what());
        }
    }

    bool ScriptValue::function_info::operator==(function_info const& rhs) const
    {
        if (paramList.size() != paramList.size())
            return false;
        for (size_t i = 0; i < paramList.size(); ++i)
        {
            if (paramList[i].value.GetValueType() != paramList[i].value.GetValueType())
                return false;
        }
        if (functionName != rhs.functionName)
            return false;
        if (className != rhs.className)
            return false;
        if (classNamespace != rhs.classNamespace)
            return false;
        return true;
    }

    /*-----------------------------------------------------------------------------*/
    /* ScriptFunctionValue                                                         */
    /*-----------------------------------------------------------------------------*/
    void ScriptValue::function_type::Invoke()
    {
        m_info.Invoke(m_objID);
    }
}