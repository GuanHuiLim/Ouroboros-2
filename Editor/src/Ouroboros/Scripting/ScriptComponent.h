#pragma once

#include <map>
#include <Utility/Hash.h>

#include "ScriptInfo.h"

namespace oo
{
    class ScriptComponent
    {
    public:
        ScriptInfo& AddScriptInfo(ScriptClassInfo const& classInfo);
        ScriptInfo* GetScriptInfo(ScriptClassInfo const& classInfo);
        void RemoveScriptInfo(ScriptClassInfo const& classInfo);

        std::map<StringHash::size_type, ScriptInfo>& GetScriptInfoAll();
        std::map<StringHash::size_type, ScriptInfo> const& GetScriptInfoAll() const;

    private:
        std::map<StringHash::size_type, ScriptInfo> scriptInfoMap;
    };
}