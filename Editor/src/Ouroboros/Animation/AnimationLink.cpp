/************************************************************************************//*!
\file           AnimationLink.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A collection of transitions between two nodes

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "pch.h"
#include "AnimationLink.h"
#include "AnimationNode.h"
#include "AnimationGroup.h"
#include "AnimationCondition.h"
#include "AnimationInternal.h"

#include <rttr/registration>

namespace oo::Anim::internal
{
	void SerializeLink(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Link& link)
	{
		writer.StartObject();
		{
			//src and dst nodes
			{
				auto ref_serialize_fn = rttr::type::get<NodeRef>().get_method(internal::serialize_method_name);
				ref_serialize_fn.invoke({}, writer, link.src, "Src Node Reference");
				ref_serialize_fn.invoke({}, writer, link.dst, "Dst Node Reference");
			}
			
			//properties
			{
				rttr::instance obj{ link };
				auto properties = rttr::type::get<Link>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					rttr::variant val{ prop.get_value(obj) };
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}			
			//conditions
			{
				auto condition_serialize_fn = rttr::type::get<Condition>().get_method(internal::serialize_method_name);
				writer.Key("Conditions", static_cast<rapidjson::SizeType>(std::string("Conditions").size()));
				writer.StartArray();
					for (auto& condition : link.conditions)
					{
						condition_serialize_fn.invoke({}, writer, condition);
					}
				writer.EndArray();
			}
			
		}
		writer.EndObject();
	}

	void LoadLink(rapidjson::GenericObject<false, rapidjson::Value>& object, Link& link)
	{
		rttr::instance obj{ link };
		//properties
		{
			auto properties = rttr::type::get<Link>().get_properties();
			for (auto& prop : properties)
			{
				auto& value = object.FindMember(prop.get_name().data())->value;

				assert(internal::loadDataFn_map.contains(prop.get_type().get_id()));
				rttr::variant val{ internal::loadDataFn_map.at(prop.get_type().get_id())(value) };
				prop.set_value(obj, val);
			}
		}
		//conditions
		{
			auto conditions = object.FindMember("Conditions")->value.GetArray();
			auto load_fn = rttr::type::get<Condition>().get_method(internal::load_method_name);
			for (auto& condition : conditions)
			{
				Condition new_condition{}; 
				load_fn.invoke({}, condition.GetObj(), new_condition);

				link.conditions.emplace_back(std::move(new_condition));
			}
		}
	}
}

namespace oo::Anim
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
	registration::class_<Link>("Animation Link")
		.property("has_exit_time", &Link::has_exit_time)
		.property("exit_time", &Link::exit_time)
		.property("fixed_duration", &Link::fixed_duration)
		.property("transition_duration", &Link::transition_duration)
		.property("transition_offset", &Link::transition_offset)
		.property("name", &Link::name)
		.property("linkID", &Link::linkID)
		.property("src", &Link::src)
		.property("dst", &Link::dst)
		.method(internal::serialize_method_name, &internal::SerializeLink)
		.method(internal::load_method_name, &internal::LoadLink)
		;
	}


	Link::SerializeFn*  Link::serializeFn =
		[](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Link& link)
	{
		
	};

	Link::Link(NodeRef _src, NodeRef _dst)
		: src{ _src },
		dst{ _dst },
		linkID{ internal::generateUID() }
	{

	}
}