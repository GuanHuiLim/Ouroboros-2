/************************************************************************************//*!
\file          SerializerScriptingProperties.h


\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Declarations of  SerializerScriptingSaveProperties & 
			   SerializerScriptingLoadProperties

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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