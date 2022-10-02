/************************************************************************************//*!
\file           ScriptComponent.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Declares the ECS scripting component that stores all the script information
                needed to create the necessary C# script instances for a specific GameObject

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <unordered_map>

#include "ScriptInfo.h"
#include <rttr/type>
namespace oo
{
    class ScriptComponent
    {
    public:
        using map_type = std::unordered_map<std::string, ScriptInfo>;

    public:
        ScriptInfo& AddScriptInfo(ScriptClassInfo const& classInfo);
        ScriptInfo* GetScriptInfo(ScriptClassInfo const& classInfo);
        void RemoveScriptInfo(ScriptClassInfo const& classInfo);

        map_type& GetScriptInfoAll();
        map_type const& GetScriptInfoAll() const;
		RTTR_ENABLE();
    private:
        map_type scriptInfoMap;
    };
}