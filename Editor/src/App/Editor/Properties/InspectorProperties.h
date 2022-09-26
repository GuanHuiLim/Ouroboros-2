/************************************************************************************//*!
\file          InspectorProperties.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         declaration for InspectorProperties 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "UI_RTTRType.h"
#include <rttr/type.h>
#include <rttr/variant.h>
#include <rttr/property.h>
struct InspectorProperties
{
	InspectorProperties();
	std::unordered_map<UI_RTTRType::UItypes, std::function<void(rttr::property& prop,std::string& name, rttr::variant& v, bool& edited, bool& endEdit)>> m_InspectorUI;
};