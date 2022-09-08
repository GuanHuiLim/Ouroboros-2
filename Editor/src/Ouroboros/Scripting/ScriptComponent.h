#pragma once

#include <unordered_map>

#include "ScriptInfo.h"

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

    private:
        map_type scriptInfoMap;
    };
}