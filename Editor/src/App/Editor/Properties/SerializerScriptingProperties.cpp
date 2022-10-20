/************************************************************************************//*!
\file          SerializerScriptingProperties.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Define how each scripting field will be saved and loaded 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "SerializerScriptingProperties.h"
#include "SceneManagement/include/SceneManager.h"
#include "Ouroboros/Scene/Scene.h"
#include "App/Editor/Utility/ImGuiManager.h"

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
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::STRING, [](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());
			rapidjson::Value data;
			data.SetString(sfi.value.GetValue<std::string>().c_str(), doc.GetAllocator());
			val.AddMember(name, data , doc.GetAllocator());
		});
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::VECTOR2, [](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());
			rapidjson::Value data(rapidjson::kArrayType);
			auto data_value = sfi.value.GetValue<oo::ScriptValue::vec2_type>();
			data.PushBack(data_value.x, doc.GetAllocator());
			data.PushBack(data_value.y, doc.GetAllocator());
			val.AddMember(name, data, doc.GetAllocator());
		});
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::VECTOR3, [](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());
			rapidjson::Value data(rapidjson::kArrayType);
			auto data_value = sfi.value.GetValue<oo::ScriptValue::vec3_type>();
			data.PushBack(data_value.x, doc.GetAllocator());
			data.PushBack(data_value.y, doc.GetAllocator());
			data.PushBack(data_value.z, doc.GetAllocator());
			val.AddMember(name, data, doc.GetAllocator());
		});
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::ENUM, [](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());
			rapidjson::Value data(sfi.value.GetValue<oo::ScriptValue::enum_type>().index);
			val.AddMember(name, data, doc.GetAllocator());
		});
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::GAMEOBJECT, [](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());
			UUID id = sfi.value.GetValue<UUID>();
			//check if object is valid before saving.
			auto go = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->FindWithInstanceID(id);
			if (go == nullptr)
				id = -1;
			rapidjson::Value data(id.GetUUID());
			val.AddMember(name, data, doc.GetAllocator());
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
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::STRING, [](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{
			sfi.value.SetValue(std::string(val.GetString())); 
		});
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::VECTOR2, [](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{
			auto arr = val.GetArray();
            oo::ScriptValue::vec2_type vec(arr[0].GetFloat(), arr[1].GetFloat());
			//glm::vec2 vec(arr[0].GetFloat(), arr[1].GetFloat());
			sfi.value.SetValue(vec);
		});
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::VECTOR3, [](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{
			auto arr = val.GetArray();
            oo::ScriptValue::vec3_type vec(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat());
			//glm::vec3 vec(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat());
			sfi.value.SetValue(vec);
		});
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::ENUM, [](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{ 
			sfi.value.GetValue<oo::ScriptValue::enum_type>().index = val.GetUint(); 
		});
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::GAMEOBJECT, [](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{
			UUID id = val.GetUint64();
			sfi.value.SetValue(id); 
		});
}