/************************************************************************************//*!
\file          SerializerProperties.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Declarations for SerializerProperties. 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <unordered_map>
#include <functional>
#include "UI_RTTRType.h"
#include "Ouroboros/ECS/GameObject.h"
#include "rapidjson/document.h"
#include "rttr/variant.h"
#include <set>
struct SerializerSaveProperties
{
	SerializerSaveProperties();
	void Reset();
	inline static std::set<oo::AssetID> s_assetUsedThisScene;
	std::unordered_map <UI_RTTRType::UItypes, std::function<void(rapidjson::Document& doc, rapidjson::Value&, rttr::variant, rttr::property)>> m_save_commands;
};

struct SerializerLoadProperties
{
	SerializerLoadProperties();
	std::unordered_map < UI_RTTRType::UItypes, std::function<void(rttr::variant&, rapidjson::Value&&)>> m_load_commands;

};