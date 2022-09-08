#pragma once
#include <unordered_map>
#include <functional>
#include "Ouroboros/Scripting/ScriptInfo.h"
struct ScriptingProperties
{
	ScriptingProperties();

	std::unordered_map<oo::ScriptValue::type_enum, std::function<void(oo::ScriptFieldInfo&, bool& editing, bool& edited)>> m_scriptUI;
};