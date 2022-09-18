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