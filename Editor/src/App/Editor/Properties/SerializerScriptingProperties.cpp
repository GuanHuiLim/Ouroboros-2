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
			oo::UUID id = sfi.value.GetValue<oo::UUID>();
			//check if object is valid before saving.
			auto go = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->FindWithInstanceID(id);
			if (go == nullptr)
				id = -1;
			rapidjson::Value data(static_cast<uint64_t>((id.GetUUID())));
			val.AddMember(name, data, doc.GetAllocator());
		});
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::LIST, [this](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());

			rapidjson::Value arr(rapidjson::kArrayType);
			auto& list_type = sfi.value.GetValue<oo::ScriptValue::list_type>();
			auto iter = m_ScriptSave.find(list_type.type);
			for (auto& item : list_type.valueList)
			{
				rapidjson::Value tempvalue(rapidjson::kObjectType);
				oo::ScriptFieldInfo temp_sfi("", item);
				iter->second(doc, tempvalue, temp_sfi);
				//might be abit wasteful to just use it like this but its a easy and fast way to do it.
				arr.PushBack(tempvalue.MemberBegin()->value,doc.GetAllocator());
			}
			val.AddMember(name, arr, doc.GetAllocator());
		});
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::CLASS, [this](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());

			rapidjson::Value obj(rapidjson::kObjectType);
			auto& class_data = sfi.value.GetValue<oo::ScriptValue::class_type>();
			for (auto& item : class_data.infoList)
			{
				auto iter = m_ScriptSave.find(item.value.GetValueType());
				if (iter != m_ScriptSave.end())
				{
					iter->second(doc,obj, item);
				}
			}
			val.AddMember(name, obj, doc.GetAllocator());
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
			oo::UUID id = val.GetUint64();
			sfi.value.SetValue(id); 
		});
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::LIST, [this](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{
			auto &list_value = sfi.value.GetValue<oo::ScriptValue::list_type>();
			auto iter = m_ScriptLoad.find(list_value.type);
			auto arr = val.GetArray();
			int counter = 0;
			for(rapidjson::SizeType i = 0 ; i < arr.Size();++i)
				list_value.Push();//create enough size for the item
			std::string tempsfi_name = "name";
			for (auto arr_iter = arr.begin(); arr_iter != arr.end(); ++arr_iter,++counter)
			{
				oo::ScriptFieldInfo sf(tempsfi_name, list_value.valueList[counter]);
				iter->second(std::move(*arr_iter), sf);
				list_value.valueList[counter] = (sf.value);
			}
		});
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::CLASS, [this](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{
			auto& class_data = sfi.value.GetValue<oo::ScriptValue::class_type>();
			auto internal_data = val.GetObj();

			int counter = 0;
			for (auto class_values = internal_data.MemberBegin() ; class_values != internal_data.MemberEnd() ; ++class_values,++counter)
			{
				auto iter = m_ScriptLoad.find(class_data.infoList[counter].value.GetValueType());
				if(iter != m_ScriptLoad.end())
					iter->second(std::move(class_values->value), class_data.infoList[counter]);
			}
		});

}