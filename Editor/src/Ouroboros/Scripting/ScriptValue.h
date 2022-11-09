/************************************************************************************//*!
\file           ScriptValue.h
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
#pragma once

#include <variant>
#include <string>
#include <vector>

#include "Scripting/ScriptEngine.h"

#include "Ouroboros/Asset/Asset.h"
#include "Ouroboros/ECS/GameObject.h"

namespace oo
{
    // forward declaration
    struct ScriptFieldInfo;

    class ScriptValue
    {
    public:
        enum class type_enum : unsigned
        {
            EMPTY = 0,
            BOOL,
            INT,
            FLOAT,
            STRING,
            ENUM,
            VECTOR2,
            VECTOR3,
            //COLOUR,
            GAMEOBJECT,
            COMPONENT,
            ASSET,
            PREFAB,
            CLASS,
            LIST,
            FUNCTION,
        };

        // used to store the necessary info to create a C# enum
        struct enum_type
        {
        public:
            std::string name_space;
            std::string name;
            unsigned int index;

            enum_type(std::string const& namespace_, std::string const& name_, unsigned int i);

            /*********************************************************************************//*!
            \brief      Gets a list of strings representing all the options of the C# Enum

            \return     all options of the C# enum as a list of strings
            *//**********************************************************************************/
            std::vector<std::string> GetOptions() const;
        };

        // used to store the necessary info to create a C# Vector2
        struct vec2_type
        {
            float x;
            float y;
        };
        // used to store the necessary info to create a C# Vector3
        struct vec3_type
        {
            float x;
            float y;
            float z;
        };

        // used to store a reference to both C# scripts and C# component interfaces
        struct component_type
        {
        public:
            UUID m_objID;
            std::string m_namespace;
            std::string m_name;
            bool m_isScript;

            component_type() : m_objID{ UUID::Invalid }, m_isScript{ false } {};

            component_type(UUID id, std::string name_space, std::string name, bool isScript)
                : m_objID{ id }, m_namespace{ name_space }, m_name{ name }, m_isScript{ isScript }
            {};

            bool is_valid();
        };

        // used to store the necessary info to create a generic C# Asset
        struct asset_type
        {
        public:
            AssetInfo::Type type;
            Asset asset;
        };

        // used to store the necessary info to create a C# prefab reference
        struct prefab_type
        {
        public:
            std::string filePath; // local file path, will prepend prefab file path later
        };

        // used to store the necessary info to create a C# data container
        struct class_type
        {
        public:
            std::string name_space;
            std::string name;
            std::vector<ScriptFieldInfo> infoList;
        };

        // used to store the necessary info to create a C# List
        struct list_type
        {
        public:
            type_enum type;
            std::string name_space;
            std::string name;
            std::vector<ScriptValue> valueList;

            explicit list_type(ScriptValue::type_enum type, std::string const& namespace_, std::string const& name_) : type{ type }, name_space{ namespace_ }, name{ name_ } {}

            /*********************************************************************************//*!
            \brief      Adds a new element to the end of the C# list data
            *//**********************************************************************************/
            void Push();

            /*********************************************************************************//*!
            \brief      Removes an element at a specific in the C# list data

            \param      index
                    the index of the element to remove
            *//**********************************************************************************/
            void Remove(size_t index);
        };

        // used to store the necessary info for a function call to a specific function in a C# class, with parameters included
        struct function_info
        {
            std::string classNamespace;
            std::string className;
            std::string functionName;
            std::vector<ScriptFieldInfo> paramList;

            /*********************************************************************************//*!
            \brief      Resets the ScriptFunctionInfo variables to a state where no function is assigned
            *//**********************************************************************************/
            void Reset();

            /*********************************************************************************//*!
            \brief      Uses function info currently stored to invoke a specific C# script's function
                        on a given GameObject

            \param      obj
                    the GameObject to invoke the script function from
            *//**********************************************************************************/
            void Invoke(UUID uuid) const;

            /*********************************************************************************//*!
            \brief      overloads the == operator to check if 2 ScriptFunctionInfo refer to the same C# function

            \param      rhs
                    the other ScriptFunctionInfo to compare to
            *//**********************************************************************************/
            bool operator==(function_info const& rhs) const;
        };
        // used to store the necessary info for a function call to a specific function in a C# class for a specific GameObject, with parameters included
        struct function_type
        {
        public:
            UUID m_objID;
            function_info m_info;

            function_type() : m_objID{ UUID::Invalid }, m_info{} { }

            /*********************************************************************************//*!
            \brief      Uses function info currently stored to invoke a specific C# script's function
                        on the currently stored GameObject
            *//**********************************************************************************/
            void Invoke();
        };

        using value_type = std::variant<
            std::monostate,
            bool,
            int,
            float,
            std::string,
            enum_type,
            vec2_type,
            vec3_type,
            //oo::Colour,
            UUID,
            component_type,
            asset_type,
            prefab_type,
            class_type,
            list_type,
            function_type
            >;

    public:
        /*-----------------------------------------------------------------------------*/
        /* Constructors                                                                */
        /*-----------------------------------------------------------------------------*/
        ScriptValue() : value(std::monostate()) {};
        inline ScriptValue(value_type const& _value) : value(_value) {};

        /*-----------------------------------------------------------------------------*/
        /* Get/Check Type Functions                                                    */
        /*-----------------------------------------------------------------------------*/
        inline type_enum GetValueType() const
        {
            return static_cast<type_enum>(value.index());
        }

        template<typename T>
        bool IsValueType() const
        {
            return std::holds_alternative<T>(value);
        }

        inline bool IsNullType() const
        {
            return value.index() == 0;
        }

        bool IsOverridable(ScriptValue const& src);

        /*-----------------------------------------------------------------------------*/
        /* getter and setter                                                           */
        /*-----------------------------------------------------------------------------*/

        template<typename T>
        T const& GetValue() const
        {
            return std::get<T>(value);
        }

        template<typename T>
        T& GetValue()
        {
            return std::get<T>(value);
        }

        template<typename T>
        bool SetValue(T const& newValue, bool overrideType = false)
        {
            if (!overrideType && !std::holds_alternative<T>(value))
                return false;
            value = newValue;
            return true;
        }

    private:
        value_type value;

    public:
        // used to keep the various methods related to specific C# types consolidated
        struct helper_functions
        {
            std::function<void(MonoObject* obj, MonoClassField* field, ScriptValue const& value)>               SetFieldValue;
            std::function<ScriptValue(MonoObject* object, MonoClassField* field, ScriptValue const& refInfo)>   GetFieldValue;
            std::function<ScriptValue(MonoObject* object, ScriptValue const& refInfo)>                          GetObjectValue;
            std::function<void(MonoObject* list, MonoClass* elementClass, ScriptValue const& value)>            AddToList;
        };

        /*********************************************************************************//*!
        \brief      Helper function used to convert a pointer to a (C#) MonoType to its
                    corresponding ScriptValue type enum
        
        \param      type
                the MonoType* to be converted

        \return     the corresponding type_enum of the given MonoType
        *//**********************************************************************************/
        static type_enum GetType(MonoType* type);

        static helper_functions const& GetHelper(type_enum type);
        static inline void SetFieldValue(MonoObject* obj, MonoClassField* field, ScriptValue const& value)
        {
            GetHelper(value.GetValueType()).SetFieldValue(obj, field, value);
        }
        static inline ScriptValue GetFieldValue(MonoObject* object, MonoClassField* field, ScriptValue const& refInfo = ScriptValue())
        {
            type_enum type = GetType(mono_field_get_type(field));
            if (type == type_enum::EMPTY)
                return ScriptValue();
            return GetHelper(type).GetFieldValue(object, field, refInfo);
        }
        static inline ScriptValue GetObjectValue(MonoObject* object, ScriptValue const& refInfo = ScriptValue())
        {
            type_enum type = GetType(mono_class_get_type(mono_object_get_class(object)));
            if (type == type_enum::EMPTY)
                return ScriptValue();
            return GetHelper(type).GetObjectValue(object, refInfo);
        }
        static inline void AddToList(MonoObject* list, MonoClass* elementClass, ScriptValue const& value)
        {
            GetHelper(value.GetValueType()).AddToList(list, elementClass, value);
        }

    private:
        static std::map<type_enum, helper_functions> utilityMap;
    };
}