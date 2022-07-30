#include "pch.h"
#include "UI_RTTRType.h"
#include <string>
#include <Utility/UUID.h>
void UI_RTTRType::Init()
{
	types.emplace(rttr::type::get<bool>().get_id(), UItypes::BOOL_TYPE);
	types.emplace(rttr::type::get<std::string>().get_id(), UItypes::STRING_TYPE);
	types.emplace(rttr::type::get<UUID>().get_id(), UItypes::UUID_TYPE);

}
