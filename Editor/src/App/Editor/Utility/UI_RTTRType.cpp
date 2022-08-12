#include "pch.h"
#include "UI_RTTRType.h"
#include <string>
#include <Utility/UUID.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
void UI_RTTRType::Init()
{
	types.emplace(rttr::type::get<bool>().get_id(), UItypes::BOOL_TYPE);
	types.emplace(rttr::type::get<std::string>().get_id(), UItypes::STRING_TYPE);
	types.emplace(rttr::type::get<UUID>().get_id(), UItypes::UUID_TYPE);
	types.emplace(rttr::type::get<glm::vec2>().get_id(), UItypes::UUID_TYPE);
	types.emplace(rttr::type::get<glm::vec3>().get_id(), UItypes::UUID_TYPE);
}
