/************************************************************************************//*!
\file           AnimationKeyFrame.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
Keyframes of an animation

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "pch.h"
#include "AnimationKeyFrame.h"
#include "AnimationInternal.h"

#include <rttr/registration>

namespace oo::Anim::internal
{
	void SerializeKeyframe(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, KeyFrame& kf)
	{
		writer.StartObject();
		{
			//properties
			{
				rttr::instance obj{ kf };
				auto properties = rttr::type::get<KeyFrame>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					
					rttr::variant val{ prop.get_value(obj) };
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}		
		}
		writer.EndObject();
	}

	void LoadKeyframe(rapidjson::GenericObject<false, rapidjson::Value>& object, KeyFrame& kf)
	{
		rttr::instance obj{ kf };
		//properties
		{
			auto properties = rttr::type::get<KeyFrame>().get_properties();
			for (auto& prop : properties)
			{
				auto& value = object.FindMember(prop.get_name().data())->value;

				assert(internal::loadDataFn_map.contains(prop.get_type().get_id()));
				rttr::variant val{ internal::loadDataFn_map.at(prop.get_type().get_id())(value) };
				prop.set_value(obj, val);
			}
		}
	}

	void SerializeScriptEvent(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, ScriptEvent& event)
	{
		writer.StartObject();
		{
			//properties
			{
				rttr::instance obj{ event };
				auto properties = rttr::type::get<ScriptEvent>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					rttr::variant val{ prop.get_value(obj) };
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}
		}
		writer.EndObject();
	}

	void LoadScriptEvent(rapidjson::GenericObject<false, rapidjson::Value>& object, ScriptEvent& event)
	{
		rttr::instance obj{ event };
		//properties
		{
			auto properties = rttr::type::get<ScriptEvent>().get_properties();
			for (auto& prop : properties)
			{
				auto& value = object.FindMember(prop.get_name().data())->value;

				assert(internal::loadDataFn_map.contains(prop.get_type().get_id()));
				rttr::variant val{ internal::loadDataFn_map.at(prop.get_type().get_id())(value) };
				prop.set_value(obj, val);
			}
		}
	}
}

namespace oo::Anim
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<KeyFrame>("Animation KeyFrame")
			.property("data", &KeyFrame::data)
			.property("time", &KeyFrame::time)
			.method(internal::serialize_method_name, &internal::SerializeKeyframe)
			.method(internal::load_method_name, &internal::LoadKeyframe)
			;
		registration::class_<ScriptEvent>("Animation ScriptEvent")
			.property("script_function_info", &ScriptEvent::script_function_info)
			.property("time", &ScriptEvent::time)
			.method(internal::serialize_method_name, &internal::SerializeScriptEvent)
			.method(internal::load_method_name, &internal::LoadScriptEvent)
			;

	}

	KeyFrame::KeyFrame(DataType _data, float _time)
		: data{ _data },
		time{ _time }
	{

	}
}