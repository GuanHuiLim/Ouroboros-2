/************************************************************************************//*!
\file           ScriptComponent.cpp
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Defines the ECS scripting component that stores all the script information
                needed to create the necessary C# script instances for a specific GameObject

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "ScriptComponent.h"
#include <rttr/registration>
namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<ScriptComponent>("ScriptComponent");
	}

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