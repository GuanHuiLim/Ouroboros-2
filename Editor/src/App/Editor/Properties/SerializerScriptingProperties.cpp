#include "pch.h"
#include "SerializerScriptingProperties.h"


SerializerScriptingSaveProperties::SerializerScriptingSaveProperties()
{
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::BOOL, [](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());
			val.AddMember(name, sfi.value.GetValue<bool>(),doc.GetAllocator());
		});
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::FLOAT, [](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());
			val.AddMember(name, sfi.value.GetValue<float>(), doc.GetAllocator());
		});
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::INT, [](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());
			val.AddMember(name, sfi.value.GetValue<int>(), doc.GetAllocator());
		});
}

SerializerScriptingLoadProperties::SerializerScriptingLoadProperties()
{
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::BOOL, [](rapidjson::Value&& val , oo::ScriptFieldInfo& sfi) 
		{sfi.value.SetValue(val.GetBool());});
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::FLOAT, [](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{sfi.value.SetValue(val.GetFloat()); });
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::INT, [](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{sfi.value.SetValue(val.GetInt()); });
}
