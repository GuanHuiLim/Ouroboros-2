#pragma once
#include <unordered_map>
#include <functional>
#include "UI_RTTRType.h"
#include "Ouroboros/ECS/GameObject.h"
#include "rapidjson/document.h"
#include "rttr/variant.h"
struct SerializerSaveProperties
{
	SerializerSaveProperties();
	std::unordered_map <UI_RTTRType::UItypes, std::function<void(rapidjson::Document& doc, rapidjson::Value&, rttr::variant, rttr::property)>> m_save_commands;
};

struct SerializerLoadProperties
{
	SerializerLoadProperties();
	std::unordered_map < UI_RTTRType::UItypes, std::function<void(rttr::variant&, rapidjson::Value&&)>> m_load_commands;

};