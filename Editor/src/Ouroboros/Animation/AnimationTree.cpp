/************************************************************************************//*!
\file           AnimationTree.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
allows you to arrange and maintain a set of Animations and 
Animation Transitions for a gameobject object

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "AnimationTree.h"
#include "AnimationParameter.h"
#include "AnimationInternal.h"

#include <rttr/registration>

namespace oo::Anim::internal
{
	void SerializeTree(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, AnimationTree& tree)
	{
		writer.StartObject();
		{
			rttr::instance obj{ tree };
			//properties
			{
				auto properties = rttr::type::get<AnimationTree>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					rttr::variant val{ prop.get_value(obj) };
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}
			//groups
			writer.Key("Groups", static_cast<rapidjson::SizeType>(std::string("Groups").size()));
			writer.StartArray();
			{
				auto serialize_fn = rttr::type::get<Group>().get_method(internal::serialize_method_name);
				for (auto& [id, group] : tree.groups)
					serialize_fn.invoke({}, writer, group);
			}
			writer.EndArray();
			//parameters
			writer.Key("Parameters", static_cast<rapidjson::SizeType>(std::string("Parameters").size()));
			writer.StartArray();
			{
				auto serialize_fn = rttr::type::get<Parameter>().get_method(internal::serialize_method_name);
				for (auto& parameter : tree.parameters)
					serialize_fn.invoke({}, writer, parameter);
			}
			writer.EndArray();
		}
		writer.EndObject();
	}
}
namespace oo::Anim
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<AnimationTree>("Animation Tree")
			.property("name", &AnimationTree::name)
			.method(internal::serialize_method_name, &internal::SerializeTree)
			;
	}
}