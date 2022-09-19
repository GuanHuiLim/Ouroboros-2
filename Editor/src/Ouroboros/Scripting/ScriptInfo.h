/************************************************************************************//*!
\file           ScriptInfo.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           August 3, 2021
\brief          Declares the structs required to contain all the information
                needed to create a new script instance during play mode using data
                set in the editor inspector durng edit mode

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <variant>

#include "ScriptValue.h"
#include "Scripting/ScriptEngine.h"

//#include "Ouroboros/Asset/Asset.h"
//#include "Ouroboros/Asset/AssetTypes.h"
#include "Ouroboros/ECS/GameObject.h"

namespace oo
{
    struct ScriptFieldInfo
    {
    public:
        ScriptFieldInfo() = default;
        ~ScriptFieldInfo() = default;

        ScriptFieldInfo(std::string const& name, ScriptValue const& value)
            : name{ name }, value{ value }, script{ nullptr }, scriptField{ nullptr } {}

        ScriptValue TryGetRuntimeValue();
        void TrySetRuntimeValue(ScriptValue const& newValue);

        void SetScriptReference(MonoClassField* field, MonoObject* obj);

    public:
        std::string name;
        ScriptValue value;

    private:
        MonoObject* script;
        MonoClassField* scriptField;
    };

    struct ScriptClassInfo
    {
        std::string name_space;
        std::string name;

        /*-----------------------------------------------------------------------------*/
        /* Default Constructor                                                         */
        /*-----------------------------------------------------------------------------*/
        ScriptClassInfo(std::string const& _namespace, std::string const& _name) : name_space(_namespace), name(_name) {}

        ScriptClassInfo(std::string const& fullName);

        ScriptClassInfo(MonoClass* klass);

        bool IsValid() const;

        /*********************************************************************************//*!
        \brief      gets a list of the info of all public fields of the C# class
         
        \return     a vector containing the info of all public fields of the class
        *//**********************************************************************************/
        std::vector<ScriptFieldInfo> const GetScriptFieldInfoAll() const;

        /*********************************************************************************//*!
        \brief      gets the function info for a specific function in the C# class

        \param      functionName
                the name of the function to get info of
        \param      paramCount
                the number of parameters of the function to get info of

        \return     the function info of the function if found, or a default constructed ScriptFunctionInfo if not found
        *//**********************************************************************************/
        ScriptValue::function_info GetFunctionInfo(std::string const& functionName, int paramCount) const;

        /*********************************************************************************//*!
        \brief      gets a list of function info for all functions in the C# class

        \param      onlyPublic
                boolean used to control if all functions should be included in the list or only public functions
        \param      maxParamCount
                the maximum number of parameters for a function to be included

        \return     the vector containing the function info of all user defined functions in the C# class
        *//**********************************************************************************/
        std::vector<ScriptValue::function_info> GetFunctionInfoAll(bool onlyPublic = false, int maxParamCount = -1) const;

        /*********************************************************************************//*!
        \brief      gets a list of the info of all the C# classes that are used by a given
                    generic public field in the class
         
        \param      fieldName
                the name of the generic public field

        \return     a vector containing the info of all C# classes used by the given field
        *//**********************************************************************************/
        std::vector<ScriptClassInfo> const GetFieldGenericTypeParams(const char* fieldName) const;

        /*********************************************************************************//*!
        \brief      gets the list of Enum Options used by a specific Enum field

        \param      fieldName
                the name of the Enum field

        \return     a vector of strings that contain the Enum options of the Enum field
        *//**********************************************************************************/
        std::vector<std::string> const GetEnumFieldOptions(const char* fieldName) const;

        /*********************************************************************************//*!
        \brief      overloads the == operator to check if the info of two ScriptClassInfo match
         
        \param      rhs
                the other ScriptClassInfo to compare to

        \return     true if the namespace and name of the classes match, else false
        *//**********************************************************************************/
        bool operator==(ScriptClassInfo const& rhs) const;

        /*********************************************************************************//*!
        \brief      overloads the != operator to check if the info of two ScriptClassInfo do not match

        \param      rhs
                the other ScriptClassInfo to compare to

        \return     true if the namespace and name of the classes do not match, else false
        *//**********************************************************************************/
        bool operator!=(ScriptClassInfo const& rhs) const;

        /*********************************************************************************//*!
        \brief      gets a script's class info as one string (<namespace>.<name>)

        \return     the std::string containing all info in the script class
        *//**********************************************************************************/
        std::string const ToString() const;
    };

    struct ScriptInfo
    {
        ScriptClassInfo classInfo;
        std::unordered_map<std::string, ScriptFieldInfo> fieldMap;

        /*********************************************************************************//*!
        \brief      constructs a C# script's info from the given class info
         
        \param      _classInfo
                the info of the C# script's class
        *//**********************************************************************************/
        ScriptInfo(ScriptClassInfo const& _classInfo);

        /*********************************************************************************//*!
        \brief      gets a pointer to the info of a desired field of the script by name
         
        \param      fieldName
                the name of the desired field
         * 
        \return     a pointer to the desired field's info, or null if it was not found
        *//**********************************************************************************/
        ScriptFieldInfo* FindFieldInfo(std::string const& fieldName);

        /*********************************************************************************//*!
        \brief      copies the field values from another given ScriptInfo

        \param      src
                the ScriptInfo to copy field values from
         *
        \return     a reference to this
        *//**********************************************************************************/
        ScriptInfo& CopyFieldValues(ScriptInfo const& src);

        /*********************************************************************************//*!
        \brief      resets all field values in the ScriptInfo to their default values
        *//**********************************************************************************/
        void ResetFieldValues();
    };
}
