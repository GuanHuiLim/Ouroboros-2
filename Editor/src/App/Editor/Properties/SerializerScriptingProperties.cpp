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
			rapidjson::Value data(static_cast<uint64_t>(id.GetUUID()));
			val.AddMember(name, data, doc.GetAllocator());
		});
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::COMPONENT, [](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());
			oo::UUID id = sfi.value.GetValue<oo::ScriptValue::component_type>().m_objID;
			//check if object is valid before saving.
			auto go = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->FindWithInstanceID(id);
			if (go == nullptr)
				id = -1;
			rapidjson::Value data(static_cast<uint64_t>(id.GetUUID()));
			//rapidjson::Value data(static_cast<uint64_t>((id.GetUUID())));//save as int64
			val.AddMember(name, data, doc.GetAllocator());
		});
	m_ScriptSave.emplace(oo::ScriptValue::type_enum::FUNCTION, [this](rapidjson::Document& doc, rapidjson::Value& val, oo::ScriptFieldInfo& sfi)
		{
			rapidjson::Value name;
			name.SetString(sfi.name.c_str(), doc.GetAllocator());
			auto function = sfi.value.GetValue<oo::ScriptValue::function_type>();
			auto id = function.m_objID;
			//check if object is valid before saving.
			auto go = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->FindWithInstanceID(id);
			if (go == nullptr)
				id = -1;

			rapidjson::Value data(rapidjson::kArrayType);
			data.PushBack(static_cast<uint64_t>((id.GetUUID())), doc.GetAllocator());
			data.PushBack(rapidjson::Value(function.m_info.classNamespace.c_str(),doc.GetAllocator()), doc.GetAllocator());
			data.PushBack(rapidjson::Value(function.m_info.className.c_str(),doc.GetAllocator()), doc.GetAllocator());
			data.PushBack(rapidjson::Value(function.m_info.functionName.c_str(), doc.GetAllocator()), doc.GetAllocator());
			data.PushBack(function.m_info.paramList.empty(), doc.GetAllocator());
			if (function.m_info.paramList.size())
			{
				auto &function_sfi = function.m_info.paramList[0];
				data.PushBack(static_cast<int64_t>(function_sfi.value.GetValueType()), doc.GetAllocator());
				//creates a temporary object to go through the saving process
				//while it can be more effecient it's a tiny cost and shouldn't be too slow.
				rapidjson::Value tempObj(rapidjson::kObjectType);
				auto iter = m_ScriptSave.find(function_sfi.value.GetValueType());
				if (iter == m_ScriptSave.end())
				{
					ASSERT_MSG(true, "type not supported?");
				}
				iter->second(doc, tempObj, function_sfi);
				data.PushBack(tempObj.MemberBegin()->value, doc.GetAllocator());
			}

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
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::COMPONENT, [](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{
			oo::UUID id = val.GetUint64();
			sfi.value.GetValue<oo::ScriptValue::component_type>().m_objID = id;
		});
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::GAMEOBJECT, [](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{
			oo::UUID id = val.GetUint64();
			sfi.value.SetValue(id);
		});
	m_ScriptLoad.emplace(oo::ScriptValue::type_enum::FUNCTION, [this](rapidjson::Value&& val, oo::ScriptFieldInfo& sfi)
		{

			/*********************************************************************************//*
			data.PushBack(static_cast<uint64_t>((id.GetUUID())), doc.GetAllocator());
			data.PushBack(rapidjson::Value(function.m_info.classNamespace.c_str(),doc.GetAllocator()), doc.GetAllocator());
			data.PushBack(rapidjson::Value(function.m_info.className.c_str(),doc.GetAllocator()), doc.GetAllocator());
			data.PushBack(rapidjson::Value(function.m_info.functionName.c_str(), doc.GetAllocator()), doc.GetAllocator());
			data.PushBack(function.m_info.paramList.empty(), doc.GetAllocator());
			*//**********************************************************************************/
			
			oo::ScriptValue::function_type function_value = sfi.value.GetValue<oo::ScriptValue::function_type>();
			auto arr = val.GetArray();
			function_value.m_objID = arr[0].GetUint64();

			function_value.m_info.classNamespace = arr[1].GetString();
			function_value.m_info.className = arr[2].GetString();
			
			oo::ScriptClassInfo classInfo{ function_value.m_info.classNamespace, function_value.m_info.className };
			std::string function_name = arr[3].GetString();
			if (function_name.empty())
				return;
			bool data = arr[4].GetBool();
			//!data because the item count is only 0 - 1 if data == true -> empty
			function_value.m_info = classInfo.GetFunctionInfo(function_name, !data);
			if (data == false)//if true means list is empty
			{
				uint64_t svalue_type = arr[5].GetUint64();
				auto iter = m_ScriptLoad.find(static_cast<oo::ScriptValue::type_enum>(svalue_type));
				if(iter == m_ScriptLoad.end())
				{
					ASSERT_MSG(true, "Why failed?");
				}

				iter->second(std::move(arr[6]), function_value.m_info.paramList[0]);

			}
			sfi.value.SetValue(function_value);
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