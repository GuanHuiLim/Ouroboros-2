/************************************************************************************//*!
\file          SerializerProperties.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Defines how each field will be saved. 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "SerializerProperties.h"
#include "Project.h"
#include <Ouroboros/Transform/TransformComponent.h>
#include <Ouroboros/Prefab/PrefabComponent.h>
#include <Ouroboros/ECS/GameObjectComponent.h>
#include "Ouroboros/Asset/Asset.h"
#include "Ouroboros/Vulkan/MeshInfo.h"
#include <Ouroboros/Vulkan/Color.h>
SerializerSaveProperties::SerializerSaveProperties()
{
	m_save_commands.emplace(UI_RTTRType::UItypes::BOOL_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		obj.AddMember(name, rapidjson::Value(variant.get_value<bool>()), doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::FLOAT_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		obj.AddMember(name, rapidjson::Value(variant.get_value<float>()), doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::INT_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		obj.AddMember(name, rapidjson::Value(variant.get_value<int>()), doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::UINT_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		obj.AddMember(name, rapidjson::Value(variant.get_value<unsigned>()), doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::DOUBLE_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		obj.AddMember(name, rapidjson::Value(variant.get_value <double> ()), doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::VEC2_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		rapidjson::Value data(rapidjson::kArrayType);
		auto vec = variant.get_value<glm::vec2>();
		data.PushBack(vec.x, doc.GetAllocator());
		data.PushBack(vec.y, doc.GetAllocator());
		obj.AddMember(name, data, doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::VEC3_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		rapidjson::Value data(rapidjson::kArrayType);
		auto vec = variant.get_value<glm::vec3>();
		data.PushBack(vec.x, doc.GetAllocator());
		data.PushBack(vec.y, doc.GetAllocator());
		data.PushBack(vec.z, doc.GetAllocator());
		obj.AddMember(name, data, doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::VEC4_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		rapidjson::Value data(rapidjson::kArrayType);
		auto vec = variant.get_value<glm::vec4>();
		// IMPT NOTE: GLM vec4 Differs from glm Quat because its XYZW and Quats are WXYZ
		data.PushBack(vec.x, doc.GetAllocator());
		data.PushBack(vec.y, doc.GetAllocator());
		data.PushBack(vec.z, doc.GetAllocator());
		data.PushBack(vec.w, doc.GetAllocator());
		obj.AddMember(name, data, doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::COLOR_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		rapidjson::Value data(rapidjson::kArrayType);
		auto vec = variant.get_value<oo::Color>();
		// IMPT NOTE: GLM vec4 Differs from glm Quat because its XYZW and Quats are WXYZ
		data.PushBack(vec.r, doc.GetAllocator());
		data.PushBack(vec.g, doc.GetAllocator());
		data.PushBack(vec.b, doc.GetAllocator());
		data.PushBack(vec.a, doc.GetAllocator());
		obj.AddMember(name, data, doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::QUAT_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		rapidjson::Value data(rapidjson::kArrayType);
		auto vec = variant.get_value<quaternion>().value;
		// IMPT NOTE: GLM vec4 Differs from glm Quat because its XYZW and Quats are WXYZ
		data.PushBack(vec.w, doc.GetAllocator());
		data.PushBack(vec.x, doc.GetAllocator());
		data.PushBack(vec.y, doc.GetAllocator());
		data.PushBack(vec.z, doc.GetAllocator());
		obj.AddMember(name, data, doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::STRING_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		rapidjson::Value v;
		std::string val = variant.to_string();
		v.SetString(val.c_str(), static_cast<rapidjson::SizeType>(val.size()), doc.GetAllocator());
		obj.AddMember(name, v, doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::PATH_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		rapidjson::Value v;
		std::string val = variant.get_value<std::filesystem::path>().string();
		v.SetString(val.c_str(), static_cast<rapidjson::SizeType>(val.size()), doc.GetAllocator());
		obj.AddMember(name, v, doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::ASSET_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		obj.AddMember(name, variant.get_value<oo::Asset>().GetID(), doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::MESH_INFO_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		auto bitset = variant.get_value<MeshInfo>().submeshBits.to_ullong();
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		obj.AddMember(name, bitset, doc.GetAllocator());
		});
}

SerializerLoadProperties::SerializerLoadProperties()
{
	m_load_commands.emplace(UI_RTTRType::UItypes::BOOL_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetBool(); });
	m_load_commands.emplace(UI_RTTRType::UItypes::INT_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetInt(); });
	m_load_commands.emplace(UI_RTTRType::UItypes::UINT_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetUint(); });

	m_load_commands.emplace(UI_RTTRType::UItypes::FLOAT_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetFloat(); });
	m_load_commands.emplace(UI_RTTRType::UItypes::DOUBLE_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetDouble(); });

	m_load_commands.emplace(UI_RTTRType::UItypes::VEC2_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {
		auto arr = val.GetArray();
		glm::vec2 v(arr[0].GetFloat(),arr[1].GetFloat());
		var = v; 
		});
	m_load_commands.emplace(UI_RTTRType::UItypes::VEC3_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {
		auto arr = val.GetArray();
		glm::vec3 v(arr[0].GetFloat(), arr[1].GetFloat(),arr[2].GetFloat());
		var = v;
		});
	m_load_commands.emplace(UI_RTTRType::UItypes::VEC4_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {
		auto arr = val.GetArray();
		/*
		* IMPT NOTE: GLM vec4 Differs from glm Quat because its XYZW and Quats are WXYZ
		auto x = arr[0].GetFloat();
		auto y = arr[1].GetFloat();
		auto z = arr[2].GetFloat();
		auto w = arr[3].GetFloat();
		glm::vec4 v(x, y, z, w);*/
		glm::vec4 v(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat(), arr[3].GetFloat());
		var = v;
		});
	m_load_commands.emplace(UI_RTTRType::UItypes::QUAT_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {
		auto arr = val.GetArray();
		/*
		* IMPT NOTE: GLM vec4 Differs from glm Quat because its XYZW and Quats are WXYZ
		auto w = arr[0].GetFloat();
		auto x = arr[1].GetFloat();
		auto y = arr[2].GetFloat();
		auto z = arr[3].GetFloat();
		glm::quat v(w, x, y, z);*/
		glm::quat v(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat(), arr[3].GetFloat());
		// Double note : quaternion does xyzw.
		var = quaternion(v);
		});

	m_load_commands.emplace(UI_RTTRType::UItypes::COLOR_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {
		auto arr = val.GetArray();
		oo::Color c;
		c.r = arr[0].GetFloat();
		c.g = arr[1].GetFloat();
		c.b = arr[2].GetFloat();
		c.a = arr[3].GetFloat();
		var = c;
		});

	m_load_commands.emplace(UI_RTTRType::UItypes::STRING_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = static_cast<std::string>(val.GetString()); });
	m_load_commands.emplace(UI_RTTRType::UItypes::PATH_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetString(); });
	m_load_commands.emplace(UI_RTTRType::UItypes::ASSET_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = Project::GetAssetManager()->Get(val.GetUint64());});
	m_load_commands.emplace(UI_RTTRType::UItypes::MESH_INFO_TYPE, [](rttr::variant& var, rapidjson::Value&& val) 
		{
			MeshInfo meshInfo;
			meshInfo.submeshBits = val.GetUint64();
			var = meshInfo;
		});

}
