#include "pch.h"
#include "UI_RTTRType.h"
#include <string>
#include <Utility/UUID.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <Quaternion/include/Quaternion.h>
#include <filesystem>
#include <Archetypes_Ecs/src/A_Ecs.h>

void UI_RTTRType::Init()
{
	types.emplace(rttr::type::get<bool>().get_id(), UItypes::BOOL_TYPE);
	types.emplace(rttr::type::get<float>().get_id(), UItypes::FLOAT_TYPE);
	types.emplace(rttr::type::get<std::string>().get_id(), UItypes::STRING_TYPE);
	types.emplace(rttr::type::get<UUID>().get_id(), UItypes::UUID_TYPE);
	types.emplace(rttr::type::get<glm::vec2>().get_id(), UItypes::VEC2_TYPE);
	types.emplace(rttr::type::get<glm::vec3>().get_id(), UItypes::VEC3_TYPE);
	types.emplace(rttr::type::get<glm::vec4>().get_id(), UItypes::VEC4_TYPE);
	types.emplace(rttr::type::get<glm::mat3>().get_id(), UItypes::MAT3_TYPE);
	types.emplace(rttr::type::get<glm::mat4>().get_id(), UItypes::MAT4_TYPE);
	types.emplace(rttr::type::get<quaternion>().get_id(), UItypes::QUAT_TYPE);
	types.emplace(rttr::type::get<std::filesystem::path>().get_id(), UItypes::PATH_TYPE);
	types.emplace(rttr::type::get<double>().get_id(), UItypes::DOUBLE_TYPE);
	types.emplace(rttr::type::get<std::size_t>().get_id(), UItypes::SIZE_T_TYPE);
	types.emplace(rttr::type::get<Ecs::EntityID>().get_id(), UItypes::ENTITY_TYPE);

}
