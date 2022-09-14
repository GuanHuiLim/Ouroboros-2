#include "pch.h"
#include "SerializerProperties.h"

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
}

SerializerLoadProperties::SerializerLoadProperties()
{
	m_load_commands.emplace(UI_RTTRType::UItypes::BOOL_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetBool(); });
	m_load_commands.emplace(UI_RTTRType::UItypes::STRING_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = static_cast<std::string>(val.GetString()); });
	m_load_commands.emplace(UI_RTTRType::UItypes::PATH_TYPE, [](rttr::variant& var, rapidjson::Value&& val) {var = val.GetString(); });
}
