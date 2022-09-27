/************************************************************************************//*!
\file          ScriptingProperties.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Declarations for ScriptingProperties 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <unordered_map>
#include <functional>
#include "Ouroboros/Scripting/ScriptInfo.h"
struct ScriptingProperties
{
	ScriptingProperties();

	std::unordered_map<oo::ScriptValue::type_enum, std::function<void(oo::ScriptFieldInfo&, bool& editing, bool& edited)>> m_scriptUI;
};