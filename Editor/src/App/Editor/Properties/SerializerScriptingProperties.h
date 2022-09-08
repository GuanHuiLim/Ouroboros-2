#pragma once
#include <unordered_map>
#include <functional>
#include <rapidjson/document.h>
#include "Ouroboros/Scripting/ScriptInfo.h"
struct SerializerScriptingSaveProperties
{
	SerializerScriptingSaveProperties();
	std::unordered_map<oo::ScriptValue::type_enum, std::function<void(rapidjson::Document&,rapidjson::Value&,oo::ScriptFieldInfo&)>> m_ScriptSave;
};
struct SerializerScriptingLoadProperties
{
	SerializerScriptingLoadProperties();
	std::unordered_map < oo::ScriptValue::type_enum, std::function<void(rapidjson::Value&&, oo::ScriptFieldInfo&)>> m_ScriptLoad;
};