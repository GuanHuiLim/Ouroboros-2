#include "pch.h"
#include "SerializerProperties.h"
#include "Project.h"
#include <Ouroboros/Transform/TransformComponent.h>
#include <Ouroboros/Prefab/PrefabComponent.h>
#include <Ouroboros/ECS/GameObjectComponent.h>
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
		data.PushBack(vec.x, doc.GetAllocator());
		data.PushBack(vec.y, doc.GetAllocator());
		data.PushBack(vec.z, doc.GetAllocator());
		data.PushBack(vec.w, doc.GetAllocator());
		obj.AddMember(name, data, doc.GetAllocator());
		});
	m_save_commands.emplace(UI_RTTRType::UItypes::QUAT_TYPE, [](rapidjson::Document& doc, rapidjson::Value& obj, rttr::variant variant, rttr::property p) {
		std::string temp = p.get_name().data();
		rapidjson::Value name;
		name.SetString(temp.c_str(), static_cast<rapidjson::SizeType>(temp.size()), doc.GetAllocator());
		rapidjson::Value data(rapidjson::kArrayType);
		auto vec = variant.get_value<quaternion>().value;
		data.PushBack(vec.x, doc.GetAllocator());
		data.PushBack(vec.y, doc.GetAllocator());
		data.PushBack(vec.z, doc.GetAllocator());
		data.PushBack(vec.w, doc.GetAllocator());
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
}

SerializerLoadProperties::SerializerLoadProperties()
{
	m_load_commands.emplace(UI_RTTRType::UItypes::BOOL_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetBool(); });
	m_load_commands.emplace(UI_RTTRType::UItypes::INT_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetInt(); });

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
		glm::vec4 v(arr[0].GetFloat(), arr[1].GetFloat(),arr[2].GetFloat(),arr[3].GetFloat());
		var = v;
		});
	m_load_commands.emplace(UI_RTTRType::UItypes::QUAT_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {
		auto arr = val.GetArray();
		glm::quat v(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat(), arr[3].GetFloat());
		var = quaternion(v);
		});

	m_load_commands.emplace(UI_RTTRType::UItypes::STRING_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = static_cast<std::string>(val.GetString()); });
	m_load_commands.emplace(UI_RTTRType::UItypes::PATH_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetString(); });
	m_load_commands.emplace(UI_RTTRType::UItypes::ASSET_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = Project::GetAssetManager()->Get(val.GetUint64());});

}
