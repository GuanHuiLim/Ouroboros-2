/************************************************************************************//*!
\file           AnimationTimeline.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A collection of keyframes that manipulate a single game object's component's value

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "pch.h"
#include "AnimationTimeline.h"
#include "AnimationKeyFrame.h"
#include "AnimationInternal.h"

#include <rttr/registration>

namespace oo::Anim::internal
{
	void SerializeTimeline(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Timeline& timeline)
	{
		writer.StartObject();
		{
			
			//properties
			{
				rttr::instance obj{ timeline };
				auto properties = rttr::type::get<Timeline>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					rttr::variant val{ prop.get_value(obj) };
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}		
			//children_index
			{
				writer.Key("children_index", static_cast<rapidjson::SizeType>(std::string("children_index").size()));
				writer.StartArray();
				{
					for (auto& index : timeline.children_index)
						writer.Int(index);
				}
				writer.EndArray();
			}
			//keyframes
			writer.Key("keyframes", static_cast<rapidjson::SizeType>(std::string("keyframes").size()));
			writer.StartArray();
			{
				auto serialize_fn = rttr::type::get<KeyFrame>().get_method(internal::serialize_method_name);
				for (auto& kf : timeline.keyframes)
					serialize_fn.invoke({}, writer, kf);
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
		registration::class_<Timeline>("Animation Timeline")
			.enumeration<Timeline::TYPE>("TYPE")
			(
				value("PROPERTY", Timeline::TYPE::PROPERTY),
				value("FBX_ANIM", Timeline::TYPE::FBX_ANIM),
				value("SCRIPT_EVENT", Timeline::TYPE::SCRIPT_EVENT)
			)
			.enumeration<Timeline::DATATYPE>("DATATYPE")
			(
				value("VEC3", Timeline::DATATYPE::VEC3),
				value("QUAT", Timeline::DATATYPE::QUAT),
				value("BOOL", Timeline::DATATYPE::BOOL)
			)
			
			.property("name", &Timeline::name)
			.property_readonly("type", &Timeline::type)
			.property_readonly("datatype", &Timeline::datatype)
			.property("rttr_type", &Timeline::rttr_type)
			.property("rttr_property", &Timeline::rttr_property)
			.property("component_hash", &Timeline::component_hash)
			.method(internal::serialize_method_name, &internal::SerializeTimeline)
			;
	}

	/*-------------------------------
	ProgressTracker
	-------------------------------*/
	ProgressTracker::ProgressTracker(const Timeline::TYPE _type) :
	type{ _type }
	{

	}
	ProgressTracker ProgressTracker::Create(Timeline::TYPE type)
	{
		ProgressTracker tracker{ type };
		switch (type)
		{
		case Timeline::TYPE::PROPERTY:
			tracker.updatefunction = &internal::UpdateProperty_Animation;
			break;
		case Timeline::TYPE::FBX_ANIM:
			tracker.updatefunction = &internal::UpdateFBX_Animation;
			break;
		case Timeline::TYPE::SCRIPT_EVENT:
			tracker.updatefunction = &internal::UpdateEvent;
			break;
		default:
			break;
		}

		return tracker;
	}
}